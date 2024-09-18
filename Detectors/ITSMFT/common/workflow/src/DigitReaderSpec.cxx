// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   DigitReaderSpec.cxx

#include <vector>

#include "TTree.h"

#include "Framework/ControlService.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/Logger.h"
#include "ITSMFTWorkflow/DigitReaderSpec.h"
#include "ITSMFTBase/DPLAlpideParam.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/ConstMCTruthContainer.h"
#include "DataFormatsITSMFT/PhysTrigger.h"
#include "CommonUtils/NameConf.h"
#include "CommonDataFormat/IRFrame.h"
#include "CommonUtils/IRFrameSelector.h"
#include <cassert>

using namespace o2::framework;
using namespace o2::itsmft;

namespace o2
{
namespace itsmft
{

DigitReader::DigitReader(o2::detectors::DetID id, bool useMC, bool useCalib, bool triggerOut)
{
  assert(id == o2::detectors::DetID::ITS || id == o2::detectors::DetID::MFT);
  mDetNameLC = mDetName = id.getName();
  mDigTreeName = "o2sim";

  mDigitBranchName = mDetName + mDigitBranchName;
  mDigROFBranchName = mDetName + mDigROFBranchName;
  mCalibBranchName = mDetName + mCalibBranchName;

  mDigtMCTruthBranchName = mDetName + mDigtMCTruthBranchName;
  mDigtMC2ROFBranchName = mDetName + mDigtMC2ROFBranchName;
  mTriggerOut = triggerOut;
  mUseMC = useMC;
  mUseCalib = useCalib;
  std::transform(mDetNameLC.begin(), mDetNameLC.end(), mDetNameLC.begin(), ::tolower);
}

void DigitReader::init(InitContext& ic)
{
  mFileName = o2::utils::Str::concat_string(o2::utils::Str::rectifyDirectory(ic.options().get<std::string>("input-dir")),
                                            ic.options().get<std::string>((mDetNameLC + "-digit-infile").c_str()));
  if (ic.options().hasOption("ignore-irframes") && !ic.options().get<bool>("ignore-irframes")) {
    mUseIRFrames = true;
    LOGP(info, "Set IRFRAME mode");
  }
  connectTree(mFileName);
}

void DigitReader::run(ProcessingContext& pc)
{
  const auto& tinfo = pc.services().get<o2::framework::TimingInfo>();
  if (tinfo.globalRunNumberChanged) { // new run is starting: 1st call
    // TODO: we have to find a way define CCDBInput for IRFrames mode only
    if (mOrigin == o2::header::gDataOriginITS) {
      const auto& alpideParam = o2::itsmft::DPLAlpideParam<o2::detectors::DetID::ITS>::Instance();
      mROFBiasInBC = alpideParam.roFrameBiasInBC;
      mROFLengthInBC = alpideParam.roFrameLengthInBC;
    } else {
      const auto& alpideParam = o2::itsmft::DPLAlpideParam<o2::detectors::DetID::MFT>::Instance();
      mROFBiasInBC = alpideParam.roFrameBiasInBC;
      mROFLengthInBC = alpideParam.roFrameLengthInBC;
    }
  }
  gsl::span<const o2::dataformats::IRFrame> irFrames{};
  if (mUseIRFrames) {
    irFrames = pc.inputs().get<gsl::span<o2::dataformats::IRFrame>>("driverInfo");
  }
  static o2::dataformats::IOMCTruthContainerView* plabels = nullptr;
  if (mUseMC && !plabels) {
    mTree->SetBranchAddress(mDigtMCTruthBranchName.c_str(), &plabels);
  }
  auto ent = mTree->GetReadEntry();

  if (!mUseIRFrames) {
    ent++;
    assert(ent < mTree->GetEntries()); // this should not happen
    mTree->GetEntry(ent);
    LOG(info) << mDetName << "DigitReader pushes " << mDigROFRec.size() << " ROFRecords, " << mDigits.size() << " digits at entry " << ent;
    pc.outputs().snapshot(Output{mOrigin, "DIGITSROF", 0}, mDigROFRec);
    pc.outputs().snapshot(Output{mOrigin, "DIGITS", 0}, mDigits);
    if (mUseCalib) {
      pc.outputs().snapshot(Output{mOrigin, "GBTCALIB", 0}, mCalib);
    }
    if (mTriggerOut) {
      std::vector<o2::itsmft::PhysTrigger> dummyTrig;
      pc.outputs().snapshot(Output{mOrigin, "PHYSTRIG", 0}, dummyTrig);
    }
    if (mUseMC) {
      auto& sharedlabels = pc.outputs().make<o2::dataformats::ConstMCTruthContainer<o2::MCCompLabel>>(Output{mOrigin, "DIGITSMCTR", 0});
      plabels->copyandflatten(sharedlabels);
      delete plabels;
      plabels = nullptr;
      pc.outputs().snapshot(Output{mOrigin, "DIGITSMC2ROF", 0}, mDigMC2ROFs);
    }
    if (mTree->GetReadEntry() + 1 >= mTree->GetEntries()) {
      pc.services().get<ControlService>().endOfStream();
      pc.services().get<ControlService>().readyToQuit(QuitRequest::Me);
    }
  } else { // need to select particulars IRs range, presumably from the same tree entry
    std::vector<o2::itsmft::Digit> digitsSel;
    std::vector<o2::itsmft::GBTCalibData> calibSel;
    std::vector<o2::itsmft::ROFRecord> digROFRecSel;
    std::vector<o2::itsmft::MC2ROFRecord> digMC2ROFsSel;
    o2::dataformats::MCTruthContainer<o2::MCCompLabel> digitLabelsSel;

    if (irFrames.size()) { // we assume the IRFrames are in the increasing order
      if (ent < 0) {
        ent++;
      }
      o2::utils::IRFrameSelector irfSel;
      irfSel.setSelectedIRFrames(irFrames, 0, 0, mROFBiasInBC, true);
      const auto irMin = irFrames.front().getMin();
      const auto irMax = irFrames.back().getMax();
      LOGP(info, "IRFRAME {} {}", irMin.asString(), irMax.asString());
      while (mDigROFRec.size() && ent < mTree->GetEntries()) {
        // do we need to read a new entry?
        if (ent > mTree->GetReadEntry()) {
          if (mUseMC) {
            delete plabels;
            plabels = nullptr;
            mTree->SetBranchAddress(mDigtMCTruthBranchName.c_str(), &plabels);
          }
          mTree->GetEntry(ent);
        }

        if (mDigROFRec.front().getBCData() <= irMax && mDigROFRec.back().getBCData() >= irMin) { // there is an overlap
          for (const auto& rof : mDigROFRec) {
            if (irfSel.check({rof.getBCData(), rof.getBCData() + mROFLengthInBC - 1}) != -1) {

              LOGP(info, "Adding selected ROF {}", rof.getBCData().asString());
              // fill
              break;
            }
          }
        }
        if (mDigROFRec.back().getBCData() < irMax) { // need to check the next entry
          ent++;
          continue;
        }
        break; // push collected data
      }
    }

    pc.outputs().snapshot(Output{mOrigin, "DIGITSROF", 0}, digROFRecSel);
    pc.outputs().snapshot(Output{mOrigin, "DIGITS", 0}, digitsSel);
    if (mUseCalib) {
      pc.outputs().snapshot(Output{mOrigin, "GBTCALIB", 0}, calibSel);
    }
    if (mTriggerOut) {
      std::vector<o2::itsmft::PhysTrigger> dummyTrig;
      pc.outputs().snapshot(Output{mOrigin, "PHYSTRIG", 0}, dummyTrig);
    }
    if (mUseMC) {
      auto& sharedlabels = pc.outputs().make<o2::dataformats::ConstMCTruthContainer<o2::MCCompLabel>>(Output{mOrigin, "DIGITSMCTR", 0});
      digitLabelsSel.flatten_to(sharedlabels);
      plabels->copyandflatten(sharedlabels);
      pc.outputs().snapshot(Output{mOrigin, "DIGITSMC2ROF", 0}, digMC2ROFsSel);
    }

    if (!irFrames.size() || irFrames.back().isLast()) {
      pc.services().get<ControlService>().endOfStream();
      pc.services().get<ControlService>().readyToQuit(QuitRequest::Me);
    }
  }
}

void DigitReader::connectTree(const std::string& filename)
{
  mTree.reset(nullptr); // in case it was already loaded
  mFile.reset(TFile::Open(filename.c_str()));
  assert(mFile && !mFile->IsZombie());
  mTree.reset((TTree*)mFile->Get(mDigTreeName.c_str()));
  assert(mTree);

  mTree->SetBranchAddress(mDigROFBranchName.c_str(), &mDigROFRecPtr);
  mTree->SetBranchAddress(mDigitBranchName.c_str(), &mDigitsPtr);
  if (mUseCalib) {
    if (!mTree->GetBranch(mCalibBranchName.c_str())) {
      throw std::runtime_error("GBT calibration data requested but not found in the tree");
    }
    mTree->SetBranchAddress(mCalibBranchName.c_str(), &mCalibPtr);
  }
  if (mUseMC) {
    if (!mTree->GetBranch(mDigtMC2ROFBranchName.c_str()) || !mTree->GetBranch(mDigtMCTruthBranchName.c_str())) {
      throw std::runtime_error("MC data requested but not found in the tree");
    }
    mTree->SetBranchAddress(mDigtMC2ROFBranchName.c_str(), &mDigMC2ROFsPtr);
  }
  LOG(info) << "Loaded tree from " << filename << " with " << mTree->GetEntries() << " entries";
}

DataProcessorSpec getITSDigitReaderSpec(bool useMC, bool useCalib, bool useTriggers, std::string defname)
{
  std::vector<OutputSpec> outputSpec;
  outputSpec.emplace_back("ITS", "DIGITS", 0, Lifetime::Timeframe);
  outputSpec.emplace_back("ITS", "DIGITSROF", 0, Lifetime::Timeframe);
  if (useCalib) {
    outputSpec.emplace_back("ITS", "GBTCALIB", 0, Lifetime::Timeframe);
  }
  if (useMC) {
    outputSpec.emplace_back("ITS", "DIGITSMCTR", 0, Lifetime::Timeframe);
    outputSpec.emplace_back("ITS", "DIGITSMC2ROF", 0, Lifetime::Timeframe);
  }
  if (useTriggers) {
    outputSpec.emplace_back("ITS", "PHYSTRIG", 0, Lifetime::Timeframe);
  }
  return DataProcessorSpec{
    "its-digit-reader",
    Inputs{},
    outputSpec,
    AlgorithmSpec{adaptFromTask<ITSDigitReader>(useMC, useCalib)},
    Options{
      {"its-digit-infile", VariantType::String, defname, {"Name of the input digit file"}},
      {"input-dir", VariantType::String, "none", {"Input directory"}}}};
}

DataProcessorSpec getMFTDigitReaderSpec(bool useMC, bool useCalib, bool useTriggers, std::string defname)
{
  std::vector<OutputSpec> outputSpec;
  outputSpec.emplace_back("MFT", "DIGITS", 0, Lifetime::Timeframe);
  outputSpec.emplace_back("MFT", "DIGITSROF", 0, Lifetime::Timeframe);
  if (useCalib) {
    outputSpec.emplace_back("MFT", "GBTCALIB", 0, Lifetime::Timeframe);
  }
  if (useMC) {
    outputSpec.emplace_back("MFT", "DIGITSMCTR", 0, Lifetime::Timeframe);
    outputSpec.emplace_back("MFT", "DIGITSMC2ROF", 0, Lifetime::Timeframe);
  }
  if (useTriggers) {
    outputSpec.emplace_back("MFT", "PHYSTRIG", 0, Lifetime::Timeframe);
  }
  return DataProcessorSpec{
    "mft-digit-reader",
    Inputs{},
    outputSpec,
    AlgorithmSpec{adaptFromTask<MFTDigitReader>(useMC, useCalib)},
    Options{
      {"mft-digit-infile", VariantType::String, defname, {"Name of the input digit file"}},
      {"input-dir", VariantType::String, "none", {"Input directory"}}}};
}

} // namespace itsmft
} // namespace o2

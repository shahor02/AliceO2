// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_CALIBRATION_LHCCLOCK_CALIBRATOR_H
#define O2_CALIBRATION_LHCCLOCK_CALIBRATOR_H

/// @file   LHCClockCalibratorSpec.h
/// @brief  Device to calibrate LHC clock phase using TOF data

#include "TOFCalibration/LHCClockCalibrator.h"
#include "DataFormatsTOF/CalibInfoTOF.h"
#include "Framework/Task.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/WorkflowSpec.h"


namespace o2
{
namespace calibration
{

class LHCClockCalibDevice : public o2::framework::Task
{
 public:
  void init(o2::framework::InitContext& ic) final
  {
    int minEnt = std::max(300, ic.options().get<int>("min-entries"));
    int nb = std::max(500, ic.options().get<int>("nbins"));
    int slotL = ic.options().get<int>("tf-per-slot");
    int delay = ic.options().get<int>("max-delay");
    mCalibrator = std::make_unique<o2::tof::LHCClockCalibrator>(minEnt, nb);
    mCalibrator->setSlotLength(slotL);
    mCalibrator->setMaxSlotsDelay(delay);
  }

  void run(o2::framework::ProcessingContext& pc) final
  {
    auto tfcounter = o2::header::get<o2::framework::DataProcessingHeader*>(pc.inputs().get("input").header)->startTime;
    auto data = pc.inputs().get<gsl::span<o2::dataformats::CalibInfoTOF>>("input");
    LOG(INFO) << "Processing TF " << tfcounter << " with " << data.size() << " tracks";
    mCalibrator->process( tfcounter, data );
  }

  void endOfStream(o2::framework::EndOfStreamContext& context) final
  {
    LOG(INFO) << "Finalizing calibration";
    constexpr uint64_t INFINITE_TF = 0xffffffffffffffff;
    mCalibrator->checkSlotsToFinalize(INFINITE_TF);
  }
  
 private:
  std::unique_ptr<o2::tof::LHCClockCalibrator> mCalibrator;
};

} // namespace calibration


namespace framework
{

DataProcessorSpec getLHCClockCalibDeviceSpec()
{
  return DataProcessorSpec{
    "calib-lhcclock-calibration",
    Inputs{{"input", "CLB", "CALIBDATA"}},
    Outputs{},
    AlgorithmSpec{adaptFromTask<o2::calibration::LHCClockCalibDevice>()},
    Options{
      {"tf-per-slot", VariantType::Int, 5, {"number of TFs per calibration time slot"}},
      {"max-delay",   VariantType::Int, 3, {"number of slots in past to consider"}},
      {"min-entries", VariantType::Int, 500, {"minimum number of entries to fit single time slot"}},
      {"nbins", VariantType::Int, 1000, {"number of bins for "}}}};
}

 
}
}


#endif

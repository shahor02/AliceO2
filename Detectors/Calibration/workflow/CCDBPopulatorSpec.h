// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_CALIBRATION_CCDBPOPULATOR_H
#define O2_CALIBRATION_CCDBPOPULATOR_H

/// @file   CCDBPopulator.h
/// @brief  device to populate CCDB

#include "Framework/DeviceSpec.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/WorkflowSpec.h"
#include "Framework/Task.h"
#include "Framework/CompletionPolicyHelpers.h"
#include "CCDB/BasicCCDBManager.h"
#include "CCDB/CcdbApi.h"
#include "DetectorsCalibration/CalibObjectWrapper.h"
#include "DataFormatsTOF/CalibLHCphaseTOF.h"

using CcdbManager = o2::ccdb::BasicCCDBManager;
using LhcPhase = o2::dataformats::CalibLHCphaseTOF;

namespace o2
{
namespace calibration
{
  using LHCphase = o2::dataformats::CalibLHCphaseTOF;
  using CalibObjectWrapper = o2::calibration::CalibObjectWrapper;
  using LHCphaseVector = std::vector<LHCphase>;
  using CalibObjectWrapperVector = std::vector<CalibObjectWrapper>;
  using CcdbApi = o2::ccdb::CcdbApi;
  
class CCDBPopulator : public o2::framework::Task
{
 public:
  void init(o2::framework::InitContext& ic) final
  {    
    mCCDBpath = ic.options().get<std::string>("ccdb-path");
    auto& mgr = CcdbManager::instance();
    mgr.setURL(mCCDBpath);
  }

  void run(o2::framework::ProcessingContext& pc) final
  {
    auto& mgr = CcdbManager::instance();
    CcdbApi api;
    api.init(mgr.getURL());

    const auto& lhcclockoutput = pc.inputs().get<LHCphaseVector>("lhcphasecalib");
    const auto& lhcclockwrapoutput = pc.inputs().get<CalibObjectWrapperVector>("lhcphasecalibwrap");
    for (int i = 0 ; i < lhcclockoutput.size(); i++) {
      auto lhcphase = lhcclockoutput[i];
      auto lhcphasewrap = lhcclockwrapoutput[i];
      //CalibObjectWrapper* w = lhcphasepair.first();
      //LHCphase* l = lhcphasepair.second();
      api.storeAsTFileAny(&lhcphase, lhcphasewrap.getPath(), lhcphasewrap.getMetaData(), lhcphasewrap.getStartValidityTimestamp(), lhcphasewrap.getEndValidityTimestamp());
    }
  }

 private:
  std::string mCCDBpath = "http://ccdb-test.cern.ch:8080"; // CCDB path

};

} // namespace calibration

namespace framework
{

DataProcessorSpec getCCDBPopulatorDeviceSpec()
{
  std::vector<InputSpec> inputs;
  inputs.emplace_back("lhcphasecalib", o2::header::gDataOriginTOF, "LHCPHASE", 0);
  inputs.emplace_back("lhcphasecalibwrap", o2::header::gDataOriginTOF, "LHCPHASEWRAPPER", 0);

  return DataProcessorSpec{
    "ccdb-populator",
    inputs,
    Outputs{},
      AlgorithmSpec{adaptFromTask<o2::calibration::CCDBPopulator>()},
    Options{
      {"ccdb-path", VariantType::String, "http://ccdb-test.cern.ch:8080", {"Path to CCDB"}}}};
}

} // namespace framework
} // namespace o2

#endif

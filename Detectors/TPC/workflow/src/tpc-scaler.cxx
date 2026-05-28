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

/// \file   tpc-scaler.cxx
/// \author Matthias Kleiner, mkleiner@ikf.uni-frankfurt.de

#include "TPCWorkflow/TPCScalerSpec.h"
#include "CommonUtils/ConfigurableParam.h"
#include "Framework/ConfigParamSpec.h"
#include "TPCCalibration/CorrectionMapsOptions.h"
#include "GlobalTrackingWorkflowHelpers/InputHelper.h"
#include "DetectorsRaw/HBFUtilsInitializer.h"
#include "Framework/CallbacksPolicy.h"

using namespace o2::framework;

void customize(std::vector<o2::framework::CallbacksPolicy>& policies)
{
  o2::raw::HBFUtilsInitializer::addNewTimeSliceCallback(policies);
}

void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  // option allowing to set parameters
  std::vector<ConfigParamSpec> options{
    ConfigParamSpec{"configKeyValues", VariantType::String, "", {"Semicolon separated key=value strings"}},
    {"enable-M-shape-correction", VariantType::Bool, false, {"Enable M-shape distortion correction"}},
    {"disable-root-input", VariantType::Bool, false, {"disable root-files input reader (e.g. for CTP input)"}},
    {"disable-IDC-scalers", VariantType::Bool, false, {"Disable TPC scalers for space-charge distortion fluctuation correction"}}};
  o2::raw::HBFUtilsInitializer::addConfigOption(options);
  o2::tpc::CorrectionMapsOptions::addGlobalOptions(options);
  std::swap(workflowOptions, options);
}

#include "Framework/runDataProcessing.h"

WorkflowSpec defineDataProcessing(ConfigContext const& config)
{
  WorkflowSpec workflow;
  o2::conf::ConfigurableParam::updateFromString(config.options().get<std::string>("configKeyValues"));
  const auto enableMShape = config.options().get<bool>("enable-M-shape-correction");
  const auto enableIDCs = !config.options().get<bool>("disable-IDC-scalers");
  auto sclOpt = o2::tpc::CorrectionMapsOptions::parseGlobalOptions(config.options());
  if (sclOpt.requestCTPLumi) {
    o2::dataformats::GlobalTrackID::mask_t dummy, src = o2::dataformats::GlobalTrackID::getSourcesMask("CTP");
    o2::globaltracking::InputHelper::addInputSpecs(config, workflow, src, src, src, false, dummy); // clusters MC is not needed
  }
  workflow.emplace_back(o2::tpc::getTPCScalerSpec(enableIDCs, enableMShape, sclOpt));
  // configure dpl timer to inject correct firstTForbit: start from the 1st orbit of TF containing 1st sampled orbit
  o2::raw::HBFUtilsInitializer hbfIni(config, workflow);
  return workflow;
}

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

/// @file   ClusterWriterSpec.cxx

#include <vector>

#include "ITS3Workflow/ClusterWriterSpec.h"
#include "DPLUtils/MakeRootTreeWriterSpec.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include "DataFormatsITSMFT/ROFRecord.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"

using namespace o2::framework;

namespace o2
{
namespace its3
{

template <typename T>
using BranchDefinition = MakeRootTreeWriterSpec::BranchDefinition<T>;
using CompClusType = std::vector<o2::itsmft::CompClusterExt>;
using PatternsType = std::vector<unsigned char>;
using ROFrameRType = std::vector<o2::itsmft::ROFRecord>;
using LabelsType = o2::dataformats::MCTruthContainer<o2::MCCompLabel>;
using ROFRecLblT = std::vector<o2::itsmft::MC2ROFRecord>;
using namespace o2::header;

DataProcessorSpec getClusterWriterSpec(bool useMC)
{
  // Spectators for logging
  // this is only to restore the original behavior
  auto compClustersSize = std::make_shared<int>(0);
  auto compClustersSizeGetter = [compClustersSize](CompClusType const& compClusters) {
    *compClustersSize = compClusters.size();
  };
  auto logger = [compClustersSize](std::vector<o2::itsmft::ROFRecord> const& rofs) {
    LOG(info) << "ITS3ClusterWriter pulled " << *compClustersSize << " clusters, in " << rofs.size() << " RO frames";
  };
  return MakeRootTreeWriterSpec("its3-cluster-writer",
                                "o2clus_its.root",
                                MakeRootTreeWriterSpec::TreeAttributes{"o2sim", "Tree with ITS clusters"},
                                BranchDefinition<CompClusType>{InputSpec{"compclus", "ITS", "COMPCLUSTERS", 0},
                                                               "ITSClusterComp",
                                                               compClustersSizeGetter},
                                BranchDefinition<PatternsType>{InputSpec{"patterns", "ITS", "PATTERNS", 0},
                                                               "ITSClusterPatt"},
                                BranchDefinition<ROFrameRType>{InputSpec{"ROframes", "ITS", "CLUSTERSROF", 0},
                                                               "ITSClustersROF",
                                                               logger},
                                BranchDefinition<LabelsType>{InputSpec{"labels", "ITS", "CLUSTERSMCTR", 0},
                                                             "ITSClusterMCTruth",
                                                             (useMC ? 1 : 0), // one branch if mc labels enabled
                                                             ""},
                                BranchDefinition<ROFRecLblT>{InputSpec{"MC2ROframes", "ITS", "CLUSTERSMC2ROF", 0},
                                                             "ITSClustersMC2ROF",
                                                             (useMC ? 1 : 0), // one branch if mc labels enabled
                                                             ""})();
}

} // namespace its3
} // namespace o2

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

// class for extended TPC cluster info (for debugging)

#ifndef ALICEO2_TPCCLINFOEXT_H
#define ALICEO2_TPCCLINFOEXT_H
#include "DataFormatsTPC/ClusterNative.h"
#include "Rtypes.h"

namespace o2
{
namespace dataformats
{

struct TPCClusInfoExt : o2::tpc::ClusterNative {
  uint8_t sector = -1;
  uint8_t row = -1;
  int16_t nClusTRange = 0;  // number of cluster on the same padrow in TBins bracket
  int16_t nClusTPRange = 0; // number of cluster in the TBins/Pads bracket
  float getDistToEdge() const;

  ClassDefNV(TPCClusInfoExt, 1);
};

struct TPCTrackClusInfoExt {
  std::vector<TPCClusInfoExt> clInfo;
  const TPCClusInfoExt* getFirst() const { return clInfo.size() ? &clInfo.front() : nullptr; }
  const TPCClusInfoExt* getLast() const { return clInfo.size() ? &clInfo.back() : nullptr; }

  ClassDefNV(TPCTrackClusInfoExt, 1);
};

} // namespace dataformats
} // namespace o2

#endif

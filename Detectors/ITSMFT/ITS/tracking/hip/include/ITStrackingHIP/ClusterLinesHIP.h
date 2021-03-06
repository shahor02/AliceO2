// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
///
/// \file ClusterLinesHIP.h
/// \brief GPU-compliant version of ClusterLines, for the moment separated, might create a common traits for ClusterLines + later specifications for each arch, later.
/// \ author: mconcas@cern.ch

#ifndef O2_ITS_TRACKING_INCLUDE_CLUSTERLINES_HIP_H_
#define O2_ITS_TRACKING_INCLUDE_CLUSTERLINES_HIP_H_

#include <hip/hip_runtime.h>
#include "GPUCommonDef.h"
#include "ITStracking/ClusterLines.h"

namespace o2
{
namespace its
{
namespace gpu
{

struct GPUVertex final {
  GPUhd() GPUVertex() : realVertex{false}
  {
  }

  GPUhd() GPUVertex(float x, float y, float z, float eX, float eY, float eZ, int contrib) : xCoord{x},
                                                                                            yCoord{y},
                                                                                            zCoord{z},
                                                                                            errorX{eZ},
                                                                                            errorY{eY},
                                                                                            errorZ{eZ},
                                                                                            contributors{contrib},
                                                                                            realVertex{true}
  {
  }
  float xCoord;
  float yCoord;
  float zCoord;
  float errorX;
  float errorY;
  float errorZ;
  int contributors;
  unsigned char realVertex;
};

class ClusterLinesHIP final
{
 public:
  GPUd() ClusterLinesHIP(const Line& firstLine, const Line& secondLine); // poor man solution to calculate duplets' centroid
  GPUd() void computeClusterCentroid();
  GPUd() inline float* getVertex() { return mVertex; }

 private:
  float mAMatrix[6];         // AX=B
  float mBMatrix[3];         // AX=B
  int mLabels[2];            // labels
  float mVertexCandidate[3]; // vertex candidate
  float mWeightMatrix[9];    // weight matrix
  float mVertex[3];          // cluster centroid position
};

} // namespace gpu
} // namespace its
} // namespace o2
#endif /* O2_ITS_TRACKING_INCLUDE_CLUSTERLINES_HIP_H_ */
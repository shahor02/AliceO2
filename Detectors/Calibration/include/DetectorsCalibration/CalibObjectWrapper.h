// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef DETECTOR_CALIB_WRAPPER_H_
#define DETECTOR_CALIB_WRAPPER_H_

#include <Rtypes.h>
#include "Framework/Logger.h"

/// @brief Wrapper for the information complementary to a CCDB object (path,
/// metadata, startTimeValidity, endTimeValidity

namespace o2
{
namespace calibration
{
class CalibObjectWrapper
{

 public:

  enum ObjectType {
		   INVALID = -1,
		   LHCPHASE = 0
  };
  
  CalibObjectWrapper() = default;
  CalibObjectWrapper(ObjectType type,
		     std::string const& path,
		     std::map<std::string, std::string> const& metadata,
		     long startValidityTimestamp,
		     long endValidityTimestamp) : mObjectType(type), mPath(path), mMD(metadata), mStart(startValidityTimestamp), mEnd(endValidityTimestamp) {}
  ~CalibObjectWrapper() = default;

  const std::string getPath() const { return mPath; }
  void setPath(std::string path) { mPath = path; }

  std::map<std::string, std::string> getMetaData() const { return mMD; }
  void setMetaData(std::map<std::string, std::string> md) { mMD = md; }

  long getStartValidityTimestamp() const { return mStart; }
  void setStartValidityTimestamp(long start) { mStart = start; }
  
  long getEndValidityTimestamp() const { return mEnd; }
  void setEndValidityTimestamp(long end) { mEnd = end; }

  void setObjectType(ObjectType o) { mObjectType = o; }
  ObjectType getObjectType() const { return mObjectType; }
  
 private:
  std::string mPath;                      // path in the CCDB
  std::map<std::string, std::string> mMD; // metadata
  long mStart;                            // start of the validity of the object
  long mEnd;                              // end of the validity of the object
  ObjectType mObjectType;                        // object type from enum defined in this class
  
  ClassDefNV(CalibObjectWrapper, 1);
};

} // namespace calibration
} // namespace o2

#endif
  

// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "TOFCalibration/LHCClockCalibrator.h"
#include "Framework/Logger.h"
#include "MathUtils/MathBase.h"

namespace o2
{
namespace tof
{

using Slot = o2::calibration::TimeSlot<o2::tof::LHCClockDataHisto>;
using o2::math_utils::math_base::fitGaus;
using LHCphase = o2::dataformats::CalibLHCphaseTOF;
  
//_____________________________________________
LHCClockDataHisto::LHCClockDataHisto()
{
  LOG(INFO) << "Default c-tor, not to be used";
}

//_____________________________________________
void LHCClockDataHisto::fill(const gsl::span<const o2::dataformats::CalibInfoTOF> data)
{
  // fill container
  for (int i = data.size(); i--;) {
    auto dt = data[i].getDeltaTimePi();
    dt += range;
    if (dt > 0 && dt < 2 * range) {
      histo[int(dt * v2Bin)]++;
      entries++;
    }
  }
}

//_____________________________________________
void LHCClockDataHisto::merge(const LHCClockDataHisto* prev)
{
  // merge data of 2 slots
  for (int i = histo.size(); i--;) {
    histo[i] += prev->histo[i];
  }
  entries += prev->entries;
}

//_____________________________________________
void LHCClockDataHisto::print() const
{
  LOG(INFO) << entries << " entries";
}

//===================================================================

//_____________________________________________
void LHCClockCalibrator::initOutput()
{

  // Here we initialize the vector of our output objects
  
  mWrapperVector.clear();
  mLHCphaseVector.clear();
  return;

}

//_____________________________________________
void LHCClockCalibrator::finalizeSlot(Slot& slot)
{
  LOG(INFO) << "Finalize slot " << slot.getTFStart() << " <= TF <= " << slot.getTFEnd();

  o2::tof::LHCClockDataHisto* c = slot.getContainer();
  LOG(INFO) << "Finalize slot with entries " << c->getEntries();

  std::vector<float> fitValues;
  /*
  auto max_it = max_element(c->histo.begin(), c->histo.end());
  float maxvalue = *max_element(c->histo.begin(), c->histo.end());
  auto max_bin = distance(c->histo.begin(), max_it); // bin where I have the maximim
  auto maxvalueBin = -(c->range) + 1./(c->v2Bin)*(max_bin + 0.5); // center of the bin where I have the maximum
  printf("maxvalue = %f, max_bin = %ld, maxvalueBin = %f, c->v2bin = %f\n", maxvalue, max_bin, maxvalueBin, 1./(c->v2Bin));
  float fitMin = maxvalueBin - 3*500; // we fit from the value where we have the maximum minus 3 times an initial sigma of 500 ps (3 and 500 should become arguments)
  float fitMax = maxvalueBin + 2*500; // we fit up to the value where we have the maximum plus 2 times an initial sigma of 500 ps (2 and 500 should become arguments)
  // limits for BC window (-12.5:12.5 ns)
  if (fitMin < -12500)
    fitMin = -12500;
  if (fitMax > 12500)
    fitMax = 12500;

  printf("maxvalue = %f, fitMin = %f, fitMax = %f\n", maxvalue, fitMin, fitMax);
  */
  
  float* array = &c->histo[0];
  int fitres = fitGaus(c->nbins, array, -(c->range), c->range, fitValues);
  LOG(INFO) << "Fit result = " << fitres;
  if (fitres >= 0) {
    LOG(INFO) << "Mean = " <<  fitValues[1];
    LOG(INFO) << "Sigma = " << fitValues[2];
  }

  // TODO: the timestamp is now given with the TF index, but it will have
  // to become an absolute time. This is true both for the lhc phase object itself
  // and the CCDB entry
  std::map<std::string, std::string> md;
  //CalibObjectWrapper wrapper(o2::calibration::CalibObjectWrapper::ObjectType::lhcphase, "TOF/LHCphase", md, slot.getTFStart(), 99999999999999);
  LHCphase l;
  l.addLHCphase(slot.getTFStart(), fitValues[1]);
  mWrapperVector.emplace_back(o2::calibration::CalibObjectWrapper::ObjectType::LHCPHASE, "TOF/LHCphase", md, slot.getTFStart(), 99999999999999);
  mLHCphaseVector.emplace_back(l);
  
  slot.print();
}

//_____________________________________________
Slot& LHCClockCalibrator::emplaceNewSlot(bool front, TFType tstart, TFType tend)
{
  auto& cont = getSlots();
  auto& slot = front ? cont.emplace_front(tstart, tend) : cont.emplace_back(tstart, tend);
  slot.setContainer(std::make_unique<LHCClockDataHisto>(mNBins, mRange));
}

} // end namespace tof
} // end namespace o2

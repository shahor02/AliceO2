// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See https://alice-o2.web.cern.ch/ for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @copyright
/// © Copyright 2014 Copyright Holders of the ALICE O2 collaboration.
/// See https://aliceinfo.cern.ch/AliceO2 for details on the Copyright holders.
/// This software is distributed under the terms of the
/// GNU General Public License version 3 (GPL Version 3).
///
/// License text in a separate file.
///
/// In applying this license, CERN does not waive the privileges and immunities
/// granted to it by virtue of its status as an Intergovernmental Organization
/// or submit itself to any jurisdiction.

/// @file   DetID.cxx
/// @author Ruben Shahoyan
/// @brief  detector ids, masks, names class implementation

#include "DetectorsBase/DetID.h"
#include "FairLogger.h"

using namespace o2::Base;

ClassImp(DetID);


DetID::DetID(ID id) : mID(id)
{
  if (id < First || id > Last) {
    LOG(FATAL) << "Unknown detector ID: " << toInt(id) << FairLogger::endl;
  }
}

/*
const char* DetID::name[] = {
  "ITS", "TPC", "TRD", "TOF", "PHS", "CPV", "EMC", "HMP", "MFT", "MCH", "MID", "ZDC", "FIT"
};

const std::int32_t DetID::mask[] = ///< detectot masks for bitvectors
  { IDtoMask(toInt(DetID::ITS)), IDtoMask(toInt(DetID::TPC)), IDtoMask(toInt(DetID::TRD)), IDtoMask(toInt(DetID::TOF)),
    IDtoMask(toInt(DetID::PHS)), IDtoMask(toInt(DetID::CPV)), IDtoMask(toInt(DetID::EMC)), IDtoMask(toInt(DetID::HMP)),
    IDtoMask(toInt(DetID::MFT)), IDtoMask(toInt(DetID::MCH)), IDtoMask(toInt(DetID::MID)), IDtoMask(toInt(DetID::ZDC)),
    IDtoMask(toInt(DetID::FIT)) };
*/

constexpr std::array<const char[4], DetID::nDetectors> DetID::name;
constexpr std::array<std::int32_t,DetID::nDetectors> DetID::mask;

//===- lib/MC/MCSectionRGB9.cpp - RGB9 Code Section Representation --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/TargetParser/Triple.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionRGB9.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace llvm;

const char *const MCSectionRGB9::StrSectionTypeRGB9[] = {
  "ROM0",
  "ROMX",
  "VRAM",
  "SRAM",
  "WRAM0",
  "WRAMX",
  "OAM",
  "HRAM",
};

bool MCSectionRGB9::isVirtualSection() const {
  switch(getType()) {
  case VRAM:
  case SRAM:
  case WRAM0:
  case WRAMX:
  case OAM:
  case HRAM:
    return true;
  default:
    return false;
  }
}

void MCSectionRGB9::printSwitchToSection(const MCAsmInfo &MAI, const Triple &T,
                                         raw_ostream &OS,
                                         const MCExpr *Subsection) const {
  OS << "\tSECTION\t\"" << getName() << "\", ";
  OS << MCSectionRGB9::getTypeStr(getType()) << '\n';
  if(Subsection) {
    report_fatal_error("PrintSwitchToSection can't handle subsections");
  }
}

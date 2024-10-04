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

MCSectionRGB9::MCSectionRGB9(StringRef Name, SectionKind K, MCSymbol *Begin)
  : MCSection(SV_RGB9, Name, K, Begin) {
  if(K.isText() || K.isReadOnly()) {
    size_t bankstr_idx = 0;
    if(Name.starts_with(".text")) {
      bankstr_idx = 5;
    } else if(Name.starts_with(".rodata")) {
      bankstr_idx = 7;
    } else {
      report_fatal_error("read only/text section does not start with .rodata or .text");
    }
    if(Name.substr(bankstr_idx).getAsInteger(10, bank) == 0) {
      if(bank == 0) {
        Type = ROM0;
      } else {
        Type = ROMX;
      }
    } else {
      report_fatal_error("whoops");
    }
  } else if(K.isData() || K.isBSS()) {
    size_t bankstr_idx = 0;
    if(Name.starts_with(".data")) {
      bankstr_idx = 5;
    } else if(Name.starts_with(".bss")) {
      bankstr_idx = 5;
    } else {
      report_fatal_error("read/write section does not start with .data or .bss");
    }
    if(Name.substr(bankstr_idx).getAsInteger(10, bank) == 0) {
      if(bank == 0) {
        Type = WRAM0;
      } else {
        Type = WRAMX;
      }
    }
  } else {
    report_fatal_error("Unknown SectionKind");
  }
}

void MCSectionRGB9::printSwitchToSection(const MCAsmInfo &MAI, const Triple &T,
                                         raw_ostream &OS,
                                         const MCExpr *Subsection) const {
  OS << "\tSECTION FRAGMENT\t\"" << getName() << "\", ";
  OS << MCSectionRGB9::getTypeStr(getType());
  if(bank) {
    OS << ", BANK[" << bank << ']';
  }
  OS << '\n';
  if(Subsection) {
    report_fatal_error("PrintSwitchToSection can't handle subsections");
  }
}

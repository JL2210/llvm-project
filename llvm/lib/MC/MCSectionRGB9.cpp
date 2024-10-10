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
#include <cstring>

using namespace llvm;

void MCSectionRGB9::constructROMSection(StringRef Section, StringRef Name, SectionKind K) {
  if(0) { // TODO: figure out a sane way to implement this
    if(!Name.starts_with(Section) ||
        Name.substr(Section.size()).getAsInteger(10, bank) != 0) {
      report_fatal_error("could not find bank number for ROM section");
    }
    if(bank == 0) {
      Type = "ROM0";
    } else {
      Type = "ROMX";
    }
  } else {
    Type = "WRAM0";
  }
}

void MCSectionRGB9::constructRAMSection(StringRef Section, StringRef Name, SectionKind K) {
  if(0) { // TODO: figure out a sane way to implement this
    if(!Name.starts_with(Section) ||
        Name.substr(Section.size()).getAsInteger(10, bank) != 0) {
      report_fatal_error("could not find bank number for RAM section");
    }
    if(bank == 0) {
      Type = "WRAM0";
    } else {
      Type = "WRAMX";
    }
  } else {
    Type = "WRAM0";
  }
}

MCSectionRGB9::MCSectionRGB9(StringRef Name, SectionKind K, MCSymbol *Begin)
  : MCSection(SV_RGB9, Name,
// next rebase: K.isText(), /*isVirtual=*/K.isBSS(),
    K, Begin), Kind(K) {
  if(K.isText()) constructROMSection(".text", Name, K);
  else if(K.isReadOnly()) constructROMSection(".rodata", Name, K);
  else if(K.isBSS()) constructRAMSection(".bss", Name, K);
  // needs to be in ROM and then copied to RAM
  else if(K.isData()) constructROMSection(".data", Name, K);
  else report_fatal_error("Unknown SectionKind");
}

void MCSectionRGB9::printSwitchToSection(const MCAsmInfo &MAI, const Triple &T,
                                         raw_ostream &OS,
                                         const MCExpr *Subsection) const {
  OS << "\tSECTION FRAGMENT\t\"" << getName() << (Kind.isData() ? ".ro" : "") << "\", ";
  OS << Type;
  if(0 && bank) {
    OS << ", BANK[" << bank << ']';
  }
  OS << '\n';
  if(Kind.isData()) {
    OS << "\tLOAD FRAGMENT\t\"" << getName() << ".rw\", ";
    if(0 && bank) {
      OS << "WRAMX, BANK[" << bank << "]";
    } else {
      OS << "WRAM0";
    }
    OS << '\n';
  }
  if(Subsection) {
    report_fatal_error("PrintSwitchToSection can't handle subsections");
  }
}

bool MCSectionRGB9::isVirtualSection() const {
  return Kind.isBSS(); // TODO: should check Type
}

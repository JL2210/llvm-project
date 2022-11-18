//===- MCSectionRGB9.h - RGB9 Machine Code Sections -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the MCSectionRGB9 class.
//
//===----------------------------------------------------------------------===//
  
#ifndef LLVM_MC_MCSECTIONRGB9_H
#define LLVM_MC_MCSECTIONRGB9_H
  
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/SectionKind.h"

namespace llvm {

class MCSymbol;

class MCSectionRGB9 final : public MCSection {
public:
  enum SectionTypeRGB9 {
    ROM0 = 0,
    ROMX,
    VRAM,
    SRAM,
    WRAM0,
    WRAMX,
    OAM,
    HRAM,
    LastSectionType
  };

  MCSectionRGB9(StringRef Name, SectionKind K, MCSymbol *Begin)
    : MCSection(SV_RGB9, Name, K, Begin) {
    if(K.isText() || K.isReadOnly()) {
      Type = ROM0; // TODO: select between ROM0 and ROMX
    } else if(K.isBSS() || K.isCommon() || K.isData()) {
      Type = WRAM0; // TODO: select between WRAM0, WRAMX, and SRAM
    } else {
      report_fatal_error("Unknown SectionKind");
    }
  }

  void printSwitchToSection(const MCAsmInfo &MAI, const Triple &T,
                            raw_ostream &OS,
                            const MCExpr *Subsection) const override;

  SectionTypeRGB9 getType() const { return Type; }
  static const char *getTypeStr(SectionTypeRGB9 T) { return StrSectionTypeRGB9[T]; }

  bool useCodeAlign() const override { return false; }
  bool isVirtualSection() const override;

  static bool classof(const MCSection *S) { return S->getVariant() == SV_RGB9; }

private:

  SectionTypeRGB9 Type;
  static const char *const StrSectionTypeRGB9[];
}; // MCSectionRGB9

} // namespace llvm

#endif /* LLVM_MC_MCSECTIONRGB9_H */

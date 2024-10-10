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
  unsigned int bank;
  StringRef Type;
  SectionKind Kind;

  void constructROMSection(StringRef Section, StringRef Name, SectionKind K);
  void constructRAMSection(StringRef Section, StringRef Name, SectionKind K);

public:
  MCSectionRGB9(StringRef Name, SectionKind K, MCSymbol *Begin);

  void printSwitchToSection(const MCAsmInfo &MAI, const Triple &T,
                            raw_ostream &OS,
                            const MCExpr *Subsection) const override;

  StringRef getName() const { return Name; }

  bool useCodeAlign() const override { return false; }

  bool isVirtualSection() const override;

  static bool classof(const MCSection *S) { return S->getVariant() == SV_RGB9; }
}; // MCSectionRGB9

} // namespace llvm

#endif /* LLVM_MC_MCSECTIONRGB9_H */

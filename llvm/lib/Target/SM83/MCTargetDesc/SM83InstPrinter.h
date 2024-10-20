//===-- SM83InstPrinter.h - Convert SM83 MCInst to asm syntax -----*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints a SM83 MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83INSTPRINTER_H
#define LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83INSTPRINTER_H

#include "SM83MCTargetDesc.h"
#include "llvm/MC/MCInstPrinter.h"

namespace llvm {
class MCOperand;
class MCRegister;

class SM83InstPrinter : public MCInstPrinter {
  void printMCOperand(const MCOperand &MO, raw_ostream &O);

public:
  SM83InstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                  const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override;
  void printRegName(raw_ostream &O, MCRegister RegNo) const override;

  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O,
                    const char *Modifier = nullptr);
  void printCondition(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printMemOp(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  // Autogenerated by tblgen.
  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);
  static const char *getRegisterName(MCRegister RegNo);
  std::pair<const char *, uint64_t> getMnemonic(const MCInst *MI) override;
};
} // namespace llvm

#endif

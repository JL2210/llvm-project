//===-- SM83InstPrinter.cpp - Convert SM83 MCInst to asm syntax ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an SM83 MCInst to a .s file.
//
//===----------------------------------------------------------------------===//


#include "SM83InstPrinter.h"
#include "SM83BaseInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

// Include the auto-generated portion of the assembly writer.
#include "SM83GenAsmWriter.inc"

void SM83InstPrinter::printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                                const MCSubtargetInfo &STI, raw_ostream &O) {
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void SM83InstPrinter::printRegName(raw_ostream &O, unsigned RegNo) const {
  O << getRegisterName(RegNo);
}

void SM83InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O, const char *Modifier) {
  assert((Modifier == 0 || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    O << MO.getImm();
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}

void SM83InstPrinter::printCondition(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O) {
  unsigned Cond = MI->getOperand(OpNo).getImm();
  if(Cond == SM83Condition::NZ)
    O << "nz";
  else if(Cond == SM83Condition::Z)
    O << "z";
  else if(Cond == SM83Condition::NC)
    O << "nc";
  else if(Cond == SM83Condition::C)
    O << "c";
  else
    llvm_unreachable("invalid condition");
}

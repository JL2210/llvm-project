//===-- SM83AsmPrinter.cpp - SM83 LLVM assembly writer --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the SM83 assembly language.
//
//===----------------------------------------------------------------------===//

#include "SM83MCInstLower.h"
#include "TargetInfo/SM83TargetInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
class SM83AsmPrinter : public AsmPrinter {
  SM83MCInstLower MCInstLowering;

public:
  explicit SM83AsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), MCInstLowering(OutContext, *this) {
  }

  StringRef getPassName() const override { return "SM83 Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;

  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);
};
} // namespace

// Simple pseudo-instructions have their lowering (with expansion to real
// instructions) auto-generated.
#include "SM83GenMCPseudoLowering.inc"

void SM83AsmPrinter::emitInstruction(const MachineInstr *MI) {
  // Do any auto-generated pseudo lowerings.
  if (emitPseudoExpansionLowering(*OutStreamer, MI))
    return;

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" void LLVMInitializeSM83AsmPrinter() {
  RegisterAsmPrinter<SM83AsmPrinter> X(getTheSM83Target());
}

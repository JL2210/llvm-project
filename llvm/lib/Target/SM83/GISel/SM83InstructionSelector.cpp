//===- SM83InstructionSelector.cpp ----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This file implements the InstructionSelector class for SM83.
//===----------------------------------------------------------------------===//

#include "SM83InstructionSelector.h"
#include "SM83TargetMachine.h"
#include "SM83Subtarget.h"
#include "SM83RegisterBankInfo.h"

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelectorImpl.h"

#define DEBUG_TYPE "sm83-isel"

using namespace llvm;

#define GET_GLOBALISEL_PREDICATE_BITSET
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATE_BITSET

namespace {

class SM83InstructionSelector : public InstructionSelector {
public:
  SM83InstructionSelector(const SM83TargetMachine &TM,
                          const SM83Subtarget &STI,
                          const SM83RegisterBankInfo &RBI);

  bool select(MachineInstr &I) override;
  static const char *getName() { return DEBUG_TYPE; }

private:
//  const SM83TargetMachine &TM;
//  const SM83Subtarget &STI;
  const SM83InstrInfo &TII;
  const SM83RegisterInfo &TRI;
  const SM83RegisterBankInfo &RBI;

  bool selectImpl(MachineInstr &I, CodeGenCoverage &CoverageInfo) const;

#define GET_GLOBALISEL_PREDICATES_DECL
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_DECL

#define GET_GLOBALISEL_TEMPORARIES_DECL
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_DECL
};

}

static const TargetRegisterClass *
getMinClassForRegBank(const RegisterBank &RB, unsigned SizeInBits) {
  unsigned RegBankID = RB.getID();
  assert(RegBankID == SM83::GPRRegBankID);

  if (SizeInBits <= 8)
    return &SM83::GR8RegClass;

  if (SizeInBits == 16)
    return &SM83::GR16RegClass;

  return nullptr;
}

#define GET_GLOBALISEL_IMPL
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_IMPL

SM83InstructionSelector::SM83InstructionSelector(const SM83TargetMachine &TM,
                          const SM83Subtarget &STI,
                          const SM83RegisterBankInfo &RBI)
    : InstructionSelector(), /*TM(TM), STI(STI),*/ TII(*STI.getInstrInfo()),
      TRI(*STI.getRegisterInfo()), RBI(RBI),
#define GET_GLOBALISEL_PREDICATES_INIT
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_INIT
#define GET_GLOBALISEL_TEMPORARIES_INIT
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_INIT
{
}

bool SM83InstructionSelector::select(MachineInstr &I) {
  // the register allocator will handle copies
  unsigned Opcode = I.getOpcode();

  MachineBasicBlock &MBB = *I.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  if (!isPreISelGenericOpcode(Opcode)) {
    if (I.isCopy()) {
      // constrain destination register
      Register DstReg = I.getOperand(0).getReg();
      const RegisterBank &DstRegBank = *RBI.getRegBank(DstReg, MRI, TRI);
      unsigned DstSize = RBI.getSizeInBits(DstReg, MRI, TRI);
      const TargetRegisterClass *DstRC = getMinClassForRegBank(DstRegBank, DstSize);

      if (!Register::isPhysicalRegister(DstReg) &&
          !RBI.constrainGenericRegister(DstReg, *DstRC, MRI)) {
        LLVM_DEBUG(dbgs() << "Failed to constrain " << TII.getName(I.getOpcode())
                          << " operand\n");
        return false;
      }
    }

    return true;
  }

  if(selectImpl(I, *CoverageInfo))
    return true;

  return false;
}

InstructionSelector *
llvm::createSM83InstructionSelector(const SM83TargetMachine &TM,
                                    SM83Subtarget &Subtarget,
                                    SM83RegisterBankInfo &RBI) {
  return new SM83InstructionSelector(TM, Subtarget, RBI);
}

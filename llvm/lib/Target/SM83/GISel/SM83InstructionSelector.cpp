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
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelectorImpl.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"

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

  const TargetRegisterClass *getRegClass(Register R, MachineRegisterInfo &MRI) const;

private:
  const SM83InstrInfo &TII;
  const SM83RegisterInfo &TRI;
  const SM83RegisterBankInfo &RBI;

  bool selectCopy(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectImpl(MachineInstr &I, CodeGenCoverage &CoverageInfo) const;
  bool selectConstant(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectMergeValues(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectUnmergeValues(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectSignedExtend(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectCompare(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectPHI(MachineInstr &I, MachineRegisterInfo &MRI) const;

#define GET_GLOBALISEL_PREDICATES_DECL
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_DECL

#define GET_GLOBALISEL_TEMPORARIES_DECL
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_DECL
};

}

const TargetRegisterClass *
SM83InstructionSelector::getRegClass(Register R,
                                     MachineRegisterInfo &MRI) const {
  const RegisterBank &RegBank = *RBI.getRegBank(R, MRI, TRI);
  assert(RegBank.getID() == SM83::GPRRegBankID);

  unsigned Size = RBI.getSizeInBits(R, MRI, TRI);

  if(Size <= 8)
    return &SM83::GR8RegClass;

  if (Size == 16)
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
  unsigned Opcode = I.getOpcode();

  MachineBasicBlock &MBB = *I.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  if (!isPreISelGenericOpcode(Opcode)) {
    if (I.isCopy()) {
      return selectCopy(I, MRI);
    }

    return true;
  }

  if(selectImpl(I, *CoverageInfo))
    return true;

  switch(I.getOpcode()) {
  default:
    return false;
  case TargetOpcode::G_GLOBAL_VALUE:
    return selectConstant(I, MRI);
  case TargetOpcode::G_MERGE_VALUES:
    return selectMergeValues(I, MRI);
  case TargetOpcode::G_UNMERGE_VALUES:
    return selectUnmergeValues(I, MRI);
  case TargetOpcode::G_SEXT:
    return selectSignedExtend(I, MRI);
  case TargetOpcode::G_ICMP:
    return selectCompare(I, MRI);
  case TargetOpcode::G_PHI:
    return selectPHI(I, MRI);
  case TargetOpcode::G_INTTOPTR:
    return selectCopy(I, MRI);
  }
}

bool SM83InstructionSelector::selectCopy(MachineInstr &I,
                                         MachineRegisterInfo &MRI) const {
  Register DstReg = I.getOperand(0).getReg();
  const TargetRegisterClass *DstRC = getRegClass(DstReg, MRI);

  if(!DstRC) {
    LLVM_DEBUG(dbgs() << "Could not determine destination register class\n");
    return false;
  }

  if(/*I.isCopy()*/1) {
    Register SrcReg = I.getOperand(1).getReg();
    const TargetRegisterClass *SrcRC = getRegClass(SrcReg, MRI);
    if(!SrcRC) {
      LLVM_DEBUG(dbgs() << "Could not determine source register class\n");
      return false;
    }
    unsigned SrcSize = TRI.getRegSizeInBits(*SrcRC);
    unsigned DstSize = TRI.getRegSizeInBits(*DstRC);
    if(SrcSize != DstSize) {
      LLVM_DEBUG(dbgs() << "Mismatched copy sizes\n");
      return false;
    }
  }

  if (!Register::isPhysicalRegister(DstReg) &&
      !RBI.constrainGenericRegister(DstReg, *DstRC, MRI)) {
    LLVM_DEBUG(dbgs() << "Failed to constrain " << TII.getName(I.getOpcode())
                      << " operand\n");
    return false;
  }

  I.setDesc(TII.get(SM83::COPY));
  return true;
}

bool SM83InstructionSelector::selectConstant(MachineInstr &I,
                                             MachineRegisterInfo &MRI) const {
  const auto DefReg = I.getOperand(0).getReg();
  auto Ty = MRI.getType(DefReg);

  unsigned Opc = 0;
  switch(Ty.getSizeInBits()) {
  default:
    report_fatal_error("Unsupported type in selectConstant");
    return false;
  case 8:
    Opc = SM83::LDri;
    break;
  case 16:
    Opc = SM83::LDrrii;
    break;
  }

  I.setDesc(TII.get(Opc));
  return constrainSelectedInstRegOperands(I, TII, TRI, RBI);
}

bool SM83InstructionSelector::selectMergeValues(MachineInstr &I,
                                                MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_MERGE_VALUES &&
         "unexpected instruction");
  MachineIRBuilder MIB(I);
  Register DstReg = I.getOperand(0).getReg();

  assert(I.getNumOperands() == 3 &&
         MRI.getType(I.getOperand(1).getReg()) == LLT::scalar(8) &&
         MRI.getType(I.getOperand(2).getReg()) == LLT::scalar(8) &&
         "Illegal instruction");
  auto Merge = MIB.buildInstr(TargetOpcode::REG_SEQUENCE, {DstReg},
                              {I.getOperand(1), int64_t(SM83::sub_low),
                               I.getOperand(2), int64_t(SM83::sub_high)});
  if (!constrainSelectedInstRegOperands(*Merge, TII, TRI, RBI))
    return false;
  I.eraseFromParent();
  return RBI.constrainGenericRegister(DstReg, SM83::GR16RegClass, MRI);
}

bool SM83InstructionSelector::selectUnmergeValues(MachineInstr &I,
                                                  MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_UNMERGE_VALUES &&
         "unexpected instruction");
  MachineIRBuilder MIB(I);
  Register LoReg = I.getOperand(0).getReg();
  Register HiReg = I.getOperand(1).getReg();
  Register SrcReg = I.getOperand(2).getReg();

  assert(I.getNumOperands() == 3 &&
         MRI.getType(LoReg) == LLT::scalar(8) &&
         MRI.getType(HiReg) == LLT::scalar(8) &&
         MRI.getType(SrcReg) == LLT::scalar(16) &&
         "Illegal instruction");
  MIB.buildInstr(TargetOpcode::COPY, {LoReg}, {})
     .addReg(SrcReg, 0, SM83::sub_low);
  MIB.buildInstr(TargetOpcode::COPY, {HiReg}, {})
     .addReg(SrcReg, 0, SM83::sub_high);

  I.eraseFromParent();
  return RBI.constrainGenericRegister(LoReg, SM83::GR8RegClass, MRI) &&
         RBI.constrainGenericRegister(HiReg, SM83::GR8RegClass, MRI) &&
         RBI.constrainGenericRegister(SrcReg, SM83::GR16RegClass, MRI);
}

bool SM83InstructionSelector::selectSignedExtend(MachineInstr &I,
                                                 MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_SEXT &&
         "unexpected instruction");
  MachineIRBuilder MIB(I);
  Register DstReg = I.getOperand(0).getReg();
  Register SrcReg = I.getOperand(1).getReg();

  assert(I.getNumOperands() == 2 &&
         MRI.getType(DstReg) == LLT::scalar(8) &&
         MRI.getType(SrcReg) == LLT::scalar(1) &&
         "Illegal instruction");

  auto CopyToA = MIB.buildCopy(SM83::A, SrcReg);
  if (!constrainSelectedInstRegOperands(*CopyToA, TII, TRI, RBI))
    return false;

  MIB.buildInstr(SM83::RRCA)
     .addDef(SM83::A)
     .addUse(SM83::A);

  MIB.buildInstr(SM83::SBCr)
     .addDef(SM83::A)
     .addUse(SM83::A)
     .addUse(SM83::A);

  auto CopyFromA = MIB.buildCopy(DstReg, Register(SM83::A));
  if (!RBI.constrainGenericRegister(CopyFromA.getReg(0), SM83::GR8RegClass, MRI))
    return false;
  
  I.eraseFromParent();
  return true;
}

bool SM83InstructionSelector::selectCompare(MachineInstr &I,
                                            MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_ICMP &&
         "unexpected instruction");
//  Register DstReg = I.getOperand(0).getReg();
//  auto Pred = CmpInst::Predicate(I.getOperand(1).getPredicate());
  return false;
}

bool SM83InstructionSelector::selectPHI(MachineInstr &I,
                                        MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_PHI &&
         "unexpected instruction");

  I.setDesc(TII.get(TargetOpcode::PHI));

  Register DstReg = I.getOperand(0).getReg();
  const TargetRegisterClass *DstRC = getRegClass(DstReg, MRI);
  return RBI.constrainGenericRegister(DstReg, *DstRC, MRI);
}

InstructionSelector *
llvm::createSM83InstructionSelector(const SM83TargetMachine &TM,
                                    SM83Subtarget &Subtarget,
                                    SM83RegisterBankInfo &RBI) {
  return new SM83InstructionSelector(TM, Subtarget, RBI);
}

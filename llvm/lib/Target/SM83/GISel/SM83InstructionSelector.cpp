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
#include "MCTargetDesc/SM83MCTargetDesc.h"
#include "SM83InstrInfo.h"
#include "SM83RegisterBankInfo.h"
#include "SM83RegisterInfo.h"
#include "SM83Subtarget.h"
#include "SM83TargetMachine.h"

#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/RegisterBank.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/Support/CodeGenCoverage.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>

#define DEBUG_TYPE "sm83-isel"

using namespace llvm;

#define GET_GLOBALISEL_PREDICATE_BITSET
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATE_BITSET

namespace {

class SM83InstructionSelector : public InstructionSelector {
public:
  SM83InstructionSelector(const SM83TargetMachine &TM, const SM83Subtarget &STI,
                          const SM83RegisterBankInfo &RBI);

  bool select(MachineInstr &I) override;
  static const char *getName() { return DEBUG_TYPE; }

  const TargetRegisterClass *getRegClass(Register R,
                                         MachineRegisterInfo &MRI) const;

private:
  const SM83InstrInfo &TII;
  const SM83RegisterInfo &TRI;
  const SM83RegisterBankInfo &RBI;
  bool selectCopy(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectImpl(MachineInstr &I, CodeGenCoverage &CoverageInfo) const;
  bool selectConstant(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectMergeUnmergeValues(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectSignedExtend(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectCompare(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectPHI(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectGEP(MachineInstr &I, MachineRegisterInfo &MRI) const;
  [[maybe_unused]] bool selectCondBranch(MachineInstr &I, MachineRegisterInfo &MRI) const;
  bool selectCarryAdd(MachineInstr &I, MachineRegisterInfo &MRI) const;

#define GET_GLOBALISEL_PREDICATES_DECL
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_DECL

#define GET_GLOBALISEL_TEMPORARIES_DECL
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_DECL
};

} // namespace

const TargetRegisterClass *
SM83InstructionSelector::getRegClass(Register R,
                                     MachineRegisterInfo &MRI) const {
  const RegisterBank &RegBank = *RBI.getRegBank(R, MRI, TRI);
  assert(RegBank.getID() == SM83::GPRRegBankID);

  unsigned Size = RBI.getSizeInBits(R, MRI, TRI);

  if (Size <= 8)
    return &SM83::GR8RegClass;

  if (Size == 16)
    return &SM83::GR16RegClass;

  return nullptr;
}

#define GET_GLOBALISEL_IMPL
#include "SM83GenGlobalISel.inc"
#undef GET_GLOBALISEL_IMPL

SM83InstructionSelector::SM83InstructionSelector(
    const SM83TargetMachine &TM, const SM83Subtarget &STI,
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

  if (selectImpl(I, *CoverageInfo))
    return true;

  switch (I.getOpcode()) {
  default:
    return false;
  case TargetOpcode::G_CONSTANT:
  case TargetOpcode::G_GLOBAL_VALUE:
    return selectConstant(I, MRI);
  case TargetOpcode::G_MERGE_VALUES:
  case TargetOpcode::G_UNMERGE_VALUES:
    return selectMergeUnmergeValues(I, MRI);
  case TargetOpcode::G_SEXT:
    return selectSignedExtend(I, MRI);
  case TargetOpcode::G_ICMP:
    return selectCompare(I, MRI);
  case TargetOpcode::G_PHI:
    return selectPHI(I, MRI);
  case TargetOpcode::G_INTTOPTR:
  case TargetOpcode::G_PTRTOINT:
    return selectCopy(I, MRI);
  case TargetOpcode::G_PTR_ADD:
    return selectGEP(I, MRI);
  case TargetOpcode::G_BRCOND:
    return selectCondBranch(I, MRI);
  case TargetOpcode::G_UADDO:
  case TargetOpcode::G_UADDE:
  case TargetOpcode::G_USUBO:
  case TargetOpcode::G_USUBE:
    return selectCarryAdd(I, MRI);
  }
}

bool SM83InstructionSelector::selectCopy(MachineInstr &I,
                                         MachineRegisterInfo &MRI) const {
  I.setDesc(TII.get(SM83::COPY));
  return constrainSelectedInstRegOperands(I, TII, TRI, RBI);
}

bool SM83InstructionSelector::selectConstant(MachineInstr &I,
                                             MachineRegisterInfo &MRI) const {
  const auto DefReg = I.getOperand(0).getReg();
  auto Ty = MRI.getType(DefReg);

  unsigned Opc = 0;
  switch (Ty.getSizeInBits()) {
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

bool SM83InstructionSelector::selectMergeUnmergeValues(
    MachineInstr &I, MachineRegisterInfo &MRI) const {
  bool isMerge = I.getOpcode() == TargetOpcode::G_MERGE_VALUES;
  bool isUnmerge = I.getOpcode() == TargetOpcode::G_UNMERGE_VALUES;
  assert((isMerge || isUnmerge) && "unexpected instruction");

  MachineIRBuilder MIB(I);
  Register WideReg = I.getOperand(isMerge ? 0 : 2).getReg();
  Register LowReg = I.getOperand(isMerge).getReg();
  Register HighReg = I.getOperand(isMerge + 1).getReg();

  if(MRI.getType(LowReg) != LLT::scalar(8) ||
     MRI.getType(HighReg) != LLT::scalar(8) ||
     MRI.getType(WideReg) != LLT::scalar(16)) {
    LLVM_DEBUG(dbgs() << "must merge/unmerge two 8-bit values to one 16-bit one");
    return false;
  }

  assert(I.getNumOperands() == 3 && "Illegal instruction");

  if(isMerge) {
    auto Merge = MIB.buildInstr(TargetOpcode::REG_SEQUENCE, {WideReg},
                                {LowReg, int64_t(SM83::sub_low),
                                 HighReg, int64_t(SM83::sub_high)});
    if (!constrainSelectedInstRegOperands(*Merge, TII, TRI, RBI)) {
      LLVM_DEBUG(dbgs() << "failed to constrain reg sequencing instruction");
      return false;
    }
  } else if(isUnmerge) {
    auto ExtractLow = MIB.buildInstr(TargetOpcode::EXTRACT_SUBREG, {LowReg},
                                 {WideReg, int64_t(SM83::sub_low)});
    auto ExtractHigh = MIB.buildInstr(TargetOpcode::EXTRACT_SUBREG, {HighReg},
                                 {WideReg, int64_t(SM83::sub_high)});
    if (!constrainSelectedInstRegOperands(*ExtractLow, TII, TRI, RBI) ||
        !constrainSelectedInstRegOperands(*ExtractHigh, TII, TRI, RBI)) {
      LLVM_DEBUG(dbgs() << "failed to constrain unmerge");
      return false;
    }
  }

  I.eraseFromParent();
  return true;
}

bool SM83InstructionSelector::selectSignedExtend(
    MachineInstr &I, MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_SEXT && "unexpected instruction");
  MachineIRBuilder MIB(I);
  Register DstReg = I.getOperand(0).getReg();
  Register SrcReg = I.getOperand(1).getReg();

  if(MRI.getType(DstReg) != LLT::scalar(8) ||
     MRI.getType(SrcReg) != LLT::scalar(1)) {
    return false;
  }

  auto Rotate = MIB.buildInstr(SM83::RRCA)
                   .addDef(SM83::A)
                   .addDef(SM83::CF, RegState::Implicit)
                   .addUse(SrcReg);
  if (!constrainSelectedInstRegOperands(*Rotate, TII, TRI, RBI))
    return false;

  auto SubCarry = MIB.buildInstr(SM83::SBCr)
                     .addDef(DstReg)
                     .addUse(SM83::A, RegState::Undef)
                     .addUse(SM83::A, RegState::Undef)
                     .addUse(SM83::CF, RegState::Implicit);
  if (!constrainSelectedInstRegOperands(*SubCarry, TII, TRI, RBI))
    return false;

  I.eraseFromParent();
  return true;
}

bool SM83InstructionSelector::selectCompare(MachineInstr &I,
                                            MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_ICMP && "unexpected instruction");
  //  Register DstReg = I.getOperand(0).getReg();
  //  auto Pred = CmpInst::Predicate(I.getOperand(1).getPredicate());
  return false;
}

bool SM83InstructionSelector::selectPHI(MachineInstr &I,
                                        MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_PHI && "unexpected instruction");

  I.setDesc(TII.get(TargetOpcode::PHI));

  Register DstReg = I.getOperand(0).getReg();
  const TargetRegisterClass *DstRC = getRegClass(DstReg, MRI);
  return RBI.constrainGenericRegister(DstReg, *DstRC, MRI);
}

bool SM83InstructionSelector::selectGEP(MachineInstr &I,
                                        MachineRegisterInfo &MRI) const {
  assert(I.getOpcode() == TargetOpcode::G_PTR_ADD &&
         "unexpected instruction");
  MachineBasicBlock &MBB = *I.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineIRBuilder MIB(I);

  Register Dst = I.getOperand(0).getReg();
  Register Base = I.getOperand(1).getReg();

  LLVM_DEBUG(dbgs() << "trying known bits analysis for inc/dec\n");

  GISelKnownBits GKB(MF);

  auto KnownBits = GKB.getKnownBits(I.getOperand(2).getReg());
  if(KnownBits.isConstant()) {
    auto Value = KnownBits.getConstant();
    if(Value.abs().ule(4)) {
      unsigned Opc = Value.isNegative() ? SM83::DECrr : SM83::INCrr;
      auto IncDec = MIB.buildInstr(Opc).addDef(Dst).addUse(Base);
      if(!constrainSelectedInstRegOperands(*IncDec, TII, TRI, RBI)) {
        return false;
      }
      for (auto v = Value.abs(); !(--v).isZero(); ) {
        MIB.buildInstr(Opc).addDef(Dst).addUse(Dst);
      }

      return true;
    }
  }

  I.setDesc(TII.get(TargetOpcode::G_ADD));
  LLVM_DEBUG(dbgs() << "trying to select G_PTR_ADD using G_ADD\n");
  return selectImpl(I, *CoverageInfo);
}

bool SM83InstructionSelector::selectCondBranch(MachineInstr &I,
                                               MachineRegisterInfo &MRI) const {
  return false;
}

bool SM83InstructionSelector::selectCarryAdd(MachineInstr &I,
                                             MachineRegisterInfo &MRI) const {
  unsigned GOpc = I.getOpcode();
  assert(GOpc == TargetOpcode::G_UADDE ||
         GOpc == TargetOpcode::G_USUBE ||
         GOpc == TargetOpcode::G_UADDO ||
         GOpc == TargetOpcode::G_USUBO);
  bool isSub = (GOpc == TargetOpcode::G_USUBE ||
                GOpc == TargetOpcode::G_USUBO);
  bool consumesCarry = (GOpc == TargetOpcode::G_UADDE ||
                        GOpc == TargetOpcode::G_USUBE);
  MachineIRBuilder MIB(I);
  Register Dst = I.getOperand(0).getReg();
  Register OutCarry = I.getOperand(1).getReg();
  Register Src1 = I.getOperand(2).getReg();
  Register Src2 = I.getOperand(3).getReg();
  Register InCarry = 0;
  if(consumesCarry) {
    InCarry = I.getOperand(4).getReg();
    assert(MRI.getType(InCarry) == LLT::scalar(1));
  }

  assert(MRI.getType(OutCarry) == LLT::scalar(1));

  if(MRI.getType(Dst) != LLT::scalar(8) ||
     MRI.getType(Src1) != LLT::scalar(8) ||
     MRI.getType(Src2) != LLT::scalar(8)) {
    LLVM_DEBUG(dbgs() << "dst, src1, and src2 must all be bytes");
    return false;
  }

  unsigned Opc = isSub ? (consumesCarry ? SM83::SBCr : SM83::SUBr)
                       : (consumesCarry ? SM83::ADCr : SM83::ADDr);

  auto Add = MIB.buildInstr(Opc)
                .addDef(Dst)
                .addDef(OutCarry, RegState::Implicit)
                .addUse(Src1)
                .addUse(Src2);

  if(consumesCarry)
    Add.addUse(InCarry, RegState::Kill);

  I.eraseFromParent();
  return constrainSelectedInstRegOperands(*Add, TII, TRI, RBI);
}

InstructionSelector *
llvm::createSM83InstructionSelector(const SM83TargetMachine &TM,
                                    SM83Subtarget &Subtarget,
                                    SM83RegisterBankInfo &RBI) {
  return new SM83InstructionSelector(TM, Subtarget, RBI);
}

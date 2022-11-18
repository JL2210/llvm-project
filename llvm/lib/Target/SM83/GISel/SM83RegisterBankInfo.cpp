//===- SM83RegisterBankInfo ------------------------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SM83RegisterBankInfo.h"

#include "llvm/CodeGen/RegisterBank.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#define GET_TARGET_REGBANK_IMPL
#include "SM83GenRegisterBank.inc"

#include "SM83GenRegisterBankInfo.def"

using namespace llvm;

const RegisterBank &
SM83RegisterBankInfo::getRegBankFromRegClass(const TargetRegisterClass &RC,
                                             LLT Ty) const {
  // only one register bank
  return getRegBank(SM83::GPRRegBankID);
}

void SM83RegisterBankInfo::getInstrPartialMappingIdxs(
    const MachineInstr &MI, const MachineRegisterInfo &MRI,
    SmallVectorImpl<PartialMappingIdx> &OpRegBankIdx) {

  unsigned NumOperands = MI.getNumOperands();
  for (unsigned Idx = 0; Idx < NumOperands; ++Idx) {
    auto &MO = MI.getOperand(Idx);
    if (!MO.isReg())
      OpRegBankIdx[Idx] = PMI_None;
    else
      OpRegBankIdx[Idx] = getPartialMappingIdx(MRI.getType(MO.getReg()));
  }
}

bool SM83RegisterBankInfo::getInstrValueMapping(
    const MachineInstr &MI,
    const SmallVectorImpl<PartialMappingIdx> &OpRegBankIdx,
    SmallVectorImpl<const ValueMapping *> &OpdsMapping) {

  unsigned NumOperands = MI.getNumOperands();
  for (unsigned Idx = 0; Idx < NumOperands; ++Idx) {
    if (!MI.getOperand(Idx).isReg())
      continue;

    auto Mapping = getValueMapping(OpRegBankIdx[Idx], 1);
    if (!Mapping->isValid())
      return false;

    OpdsMapping[Idx] = Mapping;
  }
  return true;
}

const RegisterBankInfo::InstructionMapping &
SM83RegisterBankInfo::getInstrMapping(const MachineInstr &MI) const {
  const MachineFunction &MF = *MI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  unsigned Opc = MI.getOpcode();
  unsigned NumOperands = MI.getNumOperands();

  const auto &Mapping = getInstrMappingImpl(MI);
  if (Mapping.isValid())
    return Mapping;

  switch (Opc) {
  default:
    break;
  }

  // Track the bank of each register.
  SmallVector<PartialMappingIdx, 4> OpRegBankIdx(NumOperands);
  getInstrPartialMappingIdxs(MI, MRI, OpRegBankIdx);

  // Finally construct the computed mapping.
  SmallVector<const ValueMapping *, 8> OpdsMapping(NumOperands);
  if (!getInstrValueMapping(MI, OpRegBankIdx, OpdsMapping))
    return getInvalidInstructionMapping();

  return getInstructionMapping(DefaultMappingID, /* Cost */ 1,
                               getOperandsMapping(OpdsMapping), NumOperands);
}

RegisterBankInfo::InstructionMappings
SM83RegisterBankInfo::getInstrAlternativeMappings(
    const MachineInstr &MI) const {
  // no alternative mappings
  return InstructionMappings();
}

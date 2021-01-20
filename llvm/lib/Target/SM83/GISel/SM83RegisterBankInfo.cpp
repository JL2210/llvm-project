//===- SM83RegisterBankInfo ------------------------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SM83RegisterBankInfo.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/GlobalISel/RegisterBank.h"

#define GET_TARGET_REGBANK_IMPL
#include "SM83GenRegisterBank.inc"

#include "SM83GenRegisterBankInfo.def"

using namespace llvm;

const RegisterBank &SM83RegisterBankInfo::getRegBankFromRegClass(
    const TargetRegisterClass &RC, LLT Ty) const {
  // only one register bank
  return getRegBank(SM83::GPRRegBankID);
}

const RegisterBankInfo::InstructionMapping &SM83RegisterBankInfo::getInstrMapping(
    const MachineInstr &MI) const {
  const auto &Mapping = getInstrMappingImpl(MI);

  if (Mapping.isValid())
    return Mapping;

  const unsigned Opcode = MI.getOpcode();
  const unsigned Cost = 1;
  unsigned NumOperands = MI.getNumOperands();

  const MachineFunction &MF = *MI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();

  switch (Opcode) {
  default: { // generic mapping
    SmallVector<const ValueMapping *, 4> OpdsMapping(NumOperands);
    for (unsigned Idx = 0; Idx < NumOperands; ++Idx) {
      auto &MO = MI.getOperand(Idx);
      if (!MO.isReg() || !MO.getReg())
        continue;

      LLT Ty = MRI.getType(MO.getReg());

      auto Mapping = getValueMapping(PMI_FirstGPR, Ty.getSizeInBits());
      if (!Mapping->isValid())
        return getInvalidInstructionMapping();

      OpdsMapping[Idx] = Mapping;
    }

    return getInstructionMapping(DefaultMappingID, Cost,
                                 getOperandsMapping(OpdsMapping), NumOperands);
  }
  case TargetOpcode::G_ADD: {
    // operands are of the same type
    LLT Ty = MRI.getType(MI.getOperand(0).getReg());
    unsigned Size = Ty.getSizeInBits();
    return getInstructionMapping(DefaultMappingID, Cost,
                                 getValueMapping(PMI_FirstGPR, Size),
                                 NumOperands);
  }
  }
}

RegisterBankInfo::InstructionMappings SM83RegisterBankInfo::getInstrAlternativeMappings(
    const MachineInstr &MI) const {
  // no alternative mappings
  return InstructionMappings();
}

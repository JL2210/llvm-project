//===- SM83RegisterBankInfo ------------------------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SM83RegisterBankInfo.h"
#include "llvm/CodeGen/GlobalISel/RegisterBank.h"

#define GET_TARGET_REGBANK_IMPL
#include "SM83GenRegisterBank.inc"

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
  llvm_unreachable("invalid mapping");
}

RegisterBankInfo::InstructionMappings SM83RegisterBankInfo::getInstrAlternativeMappings(
    const MachineInstr &MI) const {
  // no alternative mappings
  return InstructionMappings();
}

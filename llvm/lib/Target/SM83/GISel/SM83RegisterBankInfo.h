//===- SM83RegisterBankInfo --------------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_GISEL_SM83REGISTERBANKINFO_H
#define LLVM_LIB_TARGET_SM83_GISEL_SM83REGISTERBANKINFO_H

#include "MCTargetDesc/SM83MCTargetDesc.h"
#include "llvm/CodeGen/GlobalISel/RegisterBankInfo.h"

#define GET_REGBANK_DECLARATIONS
#include "SM83GenRegisterBank.inc"

namespace llvm {

class SM83GenRegisterBankInfo : public RegisterBankInfo {
protected:
#define GET_TARGET_REGBANK_CLASS
#include "SM83GenRegisterBank.inc"

  enum PartialMappingIdx {
    PMI_None = -1,
    PMI_GR8,
    PMI_GR16,
    PMI_FirstGPR = PMI_GR8,
    PMI_LastGPR = PMI_GR16,
    PMI_MIN = PMI_GR8,
  };

  static const RegisterBankInfo::ValueMapping *
  getValueMapping(PartialMappingIdx RBIdx, unsigned Size);
};

class SM83RegisterBankInfo final : public SM83GenRegisterBankInfo {
public:
  SM83RegisterBankInfo(const TargetRegisterInfo &TRI)
    : SM83GenRegisterBankInfo() {}

  const RegisterBank &getRegBankFromRegClass(
      const TargetRegisterClass &RC, LLT Ty) const override;

  const InstructionMapping &getInstrMapping(
      const MachineInstr &MI) const override;

  InstructionMappings getInstrAlternativeMappings(
      const MachineInstr &MI) const override;
};
}
#endif

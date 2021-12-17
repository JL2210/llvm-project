//===-- SM83InstrInfo.h - SM83 Instruction Information ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SM83 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_SM83INSTRINFO_H
#define LLVM_LIB_TARGET_SM83_SM83INSTRINFO_H

#include "SM83RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "SM83GenInstrInfo.inc"

namespace llvm {

class SM83Subtarget;

class SM83InstrInfo : public SM83GenInstrInfo {
  const SM83Subtarget &Subtarget;
  const SM83RegisterInfo RI;

public:
  SM83InstrInfo(const SM83Subtarget &STI);

  const SM83RegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB,
                   MachineBasicBlock::iterator MI, const DebugLoc &DL,
                   MCRegister DstReg, MCRegister SrcReg,
                   bool KillSrc) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;
};

} // namespace llvm

#endif

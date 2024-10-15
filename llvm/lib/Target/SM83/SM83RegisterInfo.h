//===-- SM83RegisterInfo.h - SM83 Register Information ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SM83 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_SM83REGISTERINFO_H
#define LLVM_LIB_TARGET_SM83_SM83REGISTERINFO_H

#define GET_REGINFO_HEADER
#include "SM83GenRegisterInfo.inc"

namespace llvm {
class MachineFunction;
class Triple;

struct SM83RegisterInfo : public SM83GenRegisterInfo {
  SM83RegisterInfo(const Triple &TT);

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;

  const uint32_t *getNoPreservedMask() const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  const TargetRegisterClass *getLargestLegalSuperClass(
      const TargetRegisterClass *RC, const MachineFunction &MF) const override;

  const TargetRegisterClass *getCrossCopyRegClass(
      const TargetRegisterClass *RC) const override;

  Register getStackRegister() const;
};

} // namespace llvm

#endif

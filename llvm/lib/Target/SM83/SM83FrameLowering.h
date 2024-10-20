//===-- SM83FrameLowering.h - Define frame lowering for SM83 ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class implements target-specific bits of the TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_SM83FRAMELOWERING_H
#define LLVM_LIB_TARGET_SM83_SM83FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/Alignment.h"

namespace llvm {
class MachineFunction;
class MachineBasicBlock;
class SM83Subtarget;

class SM83FrameLowering : public TargetFrameLowering {
public:
  explicit SM83FrameLowering()
      : TargetFrameLowering(StackGrowsDown, /*StackAlignment=*/Align(1),
                            /*LocalAreaOffset=*/0, /*TransAl=*/Align(1),
                            /*StackReal=*/false) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;
  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  bool hasFPImpl(const MachineFunction &MF) const override;
};

} // namespace llvm
#endif

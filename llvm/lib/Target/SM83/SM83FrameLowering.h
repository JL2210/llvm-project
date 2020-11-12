//===-- SM83FrameLowering.h - Define frame lowering for SM83 -*- C++ -*--===//
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

namespace llvm {
class SM83Subtarget;

class SM83FrameLowering : public TargetFrameLowering {
public:
  explicit SM83FrameLowering(const SM83Subtarget &STI)
    : TargetFrameLowering(StackGrowsDown,
                          /*StackAlignment=*/Align(8),
                          /*LocalAreaOffset=*/0) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override { return false; }
};

}
#endif

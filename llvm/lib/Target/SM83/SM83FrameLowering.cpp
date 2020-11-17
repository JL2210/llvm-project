//===-- SM83FrameLowering.cpp - SM83 Frame Information --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SM83 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "SM83FrameLowering.h"
#include "SM83Subtarget.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool SM83FrameLowering::hasFP(const MachineFunction &MF) const { return false; }

void SM83FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {}

void SM83FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {}

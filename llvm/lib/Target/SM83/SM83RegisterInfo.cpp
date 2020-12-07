//===-- SM83RegisterInfo.cpp - SM83 Register Information --------*- C++ -*-===//
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

#include "SM83RegisterInfo.h"
#include "SM83FrameLowering.h"
#include "MCTargetDesc/SM83MCTargetDesc.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCRegisterInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "SM83GenRegisterInfo.inc"

using namespace llvm;

SM83RegisterInfo::SM83RegisterInfo() : SM83GenRegisterInfo(SM83::A) {}

const MCPhysReg *
SM83RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector SM83RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(SM83::PC); // PC is program counter
  Reserved.set(SM83::SP); // SP is stack pointer
  assert(checkAllSuperRegsMarked(Reserved));

  return Reserved;
}

void SM83RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  llvm_unreachable("Subroutines not supported yet!");
}

Register SM83RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return SM83::NoRegister;
}

Register SM83RegisterInfo::getStackRegister() const {
  return SM83::SP;
}

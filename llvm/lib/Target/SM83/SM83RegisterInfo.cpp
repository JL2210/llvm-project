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
#include "MCTargetDesc/SM83MCTargetDesc.h"
#include "SM83FrameLowering.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCRegisterInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "SM83GenRegisterInfo.inc"

#define DEBUG_TYPE "sm83-reginfo"

using namespace llvm;

SM83RegisterInfo::SM83RegisterInfo(const Triple &TT)
    : SM83GenRegisterInfo(SM83::A) {
  (void)TT;
}

const MCPhysReg *
SM83RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  switch (MF->getFunction().getCallingConv()) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    return CSR_SaveList;
  }
}

BitVector SM83RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(SM83::PC); // PC is program counter
  Reserved.set(SM83::SP); // SP is stack pointer
  assert(checkAllSuperRegsMarked(Reserved));

  return Reserved;
}

const uint32_t *
SM83RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const {
  switch (CC) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    return CSR_RegMask;
  }
}

const uint32_t *SM83RegisterInfo::getNoPreservedMask() const {
  return CSR_NoRegs_RegMask;
}

bool SM83RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  unsigned BasePtr = SM83::SP;
  int Offset = MF.getFrameInfo().getObjectOffset(FrameIndex);

  // Skip the saved PC
  Offset += 2;

  Offset += MF.getFrameInfo().getStackSize();

  // Fold imm into offset
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  MI.getOperand(FIOperandNum).ChangeToRegister(BasePtr, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
  return true;
}

Register SM83RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return SM83::NoRegister;
}

Register SM83RegisterInfo::getStackRegister() const { return SM83::SP; }

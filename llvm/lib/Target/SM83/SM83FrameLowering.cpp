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
#include "SM83InstrInfo.h"
#include "MCTargetDesc/SM83MCTargetDesc.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool SM83FrameLowering::hasFP(const MachineFunction &MF) const { return false; }

void SM83FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
}

void SM83FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
}

 bool SM83FrameLowering::spillCalleeSavedRegisters(
     MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
     ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
   if (CSI.empty())
     return false;
  
   DebugLoc DL = (MI == MBB.end()) ? DebugLoc() : MI->getDebugLoc();
  
   MachineFunction &MF = *MBB.getParent();
   const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  
   for (const CalleeSavedInfo &I : llvm::reverse(CSI)) {
     unsigned Reg = I.getReg();
     // Add the callee-saved register as live-in. It's killed at the spill.
     MBB.addLiveIn(Reg);
     BuildMI(MBB, MI, DL, TII.get(SM83::PUSHrr))
       .addReg(Reg, RegState::Kill);
   }
   return true;
 }
  
 bool SM83FrameLowering::restoreCalleeSavedRegisters(
     MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
     MutableArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
   if (CSI.empty())
     return false;
  
   DebugLoc DL = (MI == MBB.end()) ? DebugLoc() : MI->getDebugLoc();
  
   MachineFunction &MF = *MBB.getParent();
   const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  
   for (const CalleeSavedInfo &I : CSI)
     BuildMI(MBB, MI, DL, TII.get(SM83::POPrr), I.getReg());
  
   return true;
 }

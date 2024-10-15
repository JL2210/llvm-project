//===-- SM83InstrInfo.cpp - SM83 Instruction Information ------------------===//
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

#include "SM83InstrInfo.h"
#include "MCTargetDesc/SM83InstPrinter.h"
#include "MCTargetDesc/SM83MCTargetDesc.h"
#include "SM83RegisterInfo.h"
#include "SM83Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>

#define GET_INSTRINFO_CTOR_DTOR
#include "SM83GenInstrInfo.inc"

#define DEBUG_TYPE "sm83-instrinfo"

using namespace llvm;

SM83InstrInfo::SM83InstrInfo(const SM83Subtarget &STI)
    : SM83GenInstrInfo(), Subtarget(STI), RI(STI.getTargetTriple()) {
  (void)Subtarget;
}

void SM83InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, MCRegister DstReg,
                                MCRegister SrcReg, bool KillSrc) const {
  auto &r8 = SM83::GR8RegClass, &r16 = SM83::GR16RegClass;

  if (DstReg == SrcReg)
    return;

  if (r8.contains(DstReg, SrcReg)) {
    // easy copy
    MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, get(SM83::LDrr));
    MIB.addReg(DstReg, RegState::Define)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (r16.contains(DstReg, SrcReg)) {
    // split up
    MCRegister DstLoReg = RI.getSubReg(DstReg, SM83::sub_low);
    MCRegister SrcLoReg = RI.getSubReg(SrcReg, SM83::sub_low);
    MCRegister DstHiReg = RI.getSubReg(DstReg, SM83::sub_high);
    MCRegister SrcHiReg = RI.getSubReg(SrcReg, SM83::sub_high);
    copyPhysReg(MBB, MI, DL, DstLoReg, SrcLoReg, KillSrc);
    copyPhysReg(MBB, MI, DL, DstHiReg, SrcHiReg, KillSrc);
    --MI;                                // set attributes on last copy
    MI->addRegisterDefined(DstReg, &RI); // the destination is always defined
    if (KillSrc) {
      // the source is only killed if we're told it is
      // TODO: find out when this is even reasonable
      MI->addRegisterKilled(SrcReg, &RI, true);
    }
  } else {
    report_fatal_error("reg not in r8 or r16");
  }
}

bool SM83InstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  DebugLoc DL = MI.getDebugLoc();
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineInstrBuilder MIB(MF, MI);

  switch (MI.getOpcode()) {
  default:
    return false;
  }
  return true;
}

static int CurFrameIdx = 0;

void SM83InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DstReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI,
                                         Register VReg) const {
  LLVM_DEBUG(dbgs() << "Frame Index: " << FrameIndex);
  
  report_fatal_error("load from stack slot unimplemented");
}

void SM83InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        Register SrcReg, bool IsKill,
                                        int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI,
                                        Register VReg) const {
  LLVM_DEBUG(dbgs() << "Frame Index: " << FrameIndex);
  
  report_fatal_error("store to stack slot unimplemented");
}

bool SM83InstrInfo::isReallyTriviallyReMaterializable(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case SM83::LDri:
  case SM83::LDrrii:
    return true;
  default:
    return false;
  }
}

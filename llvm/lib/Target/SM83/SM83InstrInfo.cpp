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
#include "SM83RegisterInfo.h"
#include "SM83Subtarget.h"
#include "MCTargetDesc/SM83MCTargetDesc.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "SM83GenInstrInfo.inc"

using namespace llvm;

SM83InstrInfo::SM83InstrInfo(const SM83Subtarget &STI)
  : SM83GenInstrInfo(), Subtarget(STI), RI(STI.getTargetTriple()) {
  (void)Subtarget;
}

void SM83InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI, const DebugLoc &DL,
                                MCRegister DstReg, MCRegister SrcReg,
                                bool KillSrc) const {
  auto &r8  = SM83::GR8RegClass,
       &r16 = SM83::GR16RegClass;

  if(DstReg == SrcReg) return;

  if(r8.contains(DstReg, SrcReg)) {
    // easy copy
    MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, get(SM83::LDrr));
    MIB.addReg(DstReg, RegState::Define)
       .addReg(SrcReg, getKillRegState(KillSrc));
  } else if(r16.contains(DstReg, SrcReg)) {
    // split up
    MCRegister DstLoReg = RI.getSubReg(DstReg, SM83::sub_low);
    MCRegister SrcLoReg = RI.getSubReg(SrcReg, SM83::sub_low);
    MCRegister DstHiReg = RI.getSubReg(DstReg, SM83::sub_high);
    MCRegister SrcHiReg = RI.getSubReg(SrcReg, SM83::sub_high);
    copyPhysReg(MBB, MI, DL, DstLoReg, SrcLoReg, KillSrc);
    copyPhysReg(MBB, MI, DL, DstHiReg, SrcHiReg, KillSrc);
    --MI; // set attributes on last copy
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

  switch (unsigned Opc = MI.getOpcode()) {
#if 0
    case SM83::LDaz: {
        Register Reg = MIB.getReg(0);
        MI.setDesc(get(SM83::XORr));
        MIB.addReg(Reg, RegState::Undef).addReg(Reg, RegState::Undef);
      }
      break;
    case SM83::NEGA: {
        Register Reg = MIB.getReg(0);
        BuildMI(MBB, MI, DL, get(SM83::CPL), Reg)
          .addReg(Reg);
        MI.setDesc(get(SM83::INCr));
        MIB.addReg(Reg);
      }
      break;
    case SM83::CPLrr: { // defs = a
        Register DstReg = MIB.getReg(0);
        Register SrcReg = MIB.getReg(1);
        bool KillSrc = (DstReg == SrcReg);
        MCRegister DstLoReg = RI.getSubReg(DstReg, SM83::sub_low);
        MCRegister SrcLoReg = RI.getSubReg(SrcReg, SM83::sub_low);
        MCRegister DstHiReg = RI.getSubReg(DstReg, SM83::sub_high);
        MCRegister SrcHiReg = RI.getSubReg(SrcReg, SM83::sub_high);

        copyPhysReg(MBB, MI, DL, SM83::A, SrcLoReg, /*KillSrc=*/false);
        BuildMI(MBB, MI, DL, get(SM83::CPL), SM83::A)
          .addReg(SM83::A, RegState::ImplicitDefine);
	copyPhysReg(MBB, MI, DL, DstLoReg, SM83::A, /*KillSrc=*/false);

        copyPhysReg(MBB, MI, DL, SM83::A, SrcHiReg, /*KillSrc=*/false);
        BuildMI(MBB, MI, DL, get(SM83::CPL), SM83::A)
          .addReg(SM83::A, RegState::ImplicitDefine);
        copyPhysReg(MBB, MI, DL, DstHiReg, SM83::A, /*KillSrc=*/false);

        MBB.back().addRegisterDefined(DstReg, &RI);
        if (KillSrc)
          MBB.back().addRegisterKilled(SrcReg, &RI, true);
        MI.eraseFromParent();
      }
      break;
#endif /* 0 */
    default:
      return false;
  }
  return true;
}

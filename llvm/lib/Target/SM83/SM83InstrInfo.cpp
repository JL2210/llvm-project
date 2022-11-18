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
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"

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

static unsigned pseudoRIToAI(unsigned Opc) {
  using namespace SM83;
  switch (Opc) {
  default:
    llvm_unreachable("cannot convert reg-imm to a-imm");
  case XORri:
    return XORi;
  case ADDri:
    return ADDi;
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
    BuildMI(MBB, MI, DL, get(SM83::XORr), SM83::A)
      .addReg(SM83::A, RegState::Undef)
      .addReg(SM83::A, RegState::Undef);
    MI.eraseFromParent();
  }
  break;
#endif
  case SM83::XORri:
  case SM83::ADDri: {
    Register DstReg = MI.getOperand(0).getReg();
    auto &SrcRegOp = MI.getOperand(1);
    Register SrcReg = SrcRegOp.getReg();
    unsigned Opc = pseudoRIToAI(MI.getOpcode());
    uint8_t imm = MI.getOperand(2).getImm();

    copyPhysReg(MBB, MI, DL, SM83::A, SrcReg, /*KillSrc=*/SrcRegOp.isKill());
    BuildMI(MBB, MI, DL, get(Opc), SM83::A).addReg(SM83::A).addImm(imm);
    copyPhysReg(MBB, MI, DL, DstReg, SM83::A, /*KillSrc=*/true);
    MI.eraseFromParent();
  } break;
  case SM83::FakeLEA: {
    assert(MI.getOperand(1).isReg() && "expected global!");
    assert(MI.getOperand(2).isImm() && "expected immediate offset!");
  } break;
  default:
    return false;
  }
  return true;
}

static unsigned getLoadStoreRegOpc(const TargetRegisterClass *RC,
                                   const TargetRegisterInfo *TRI, bool load) {
  switch (TRI->getRegSizeInBits(*RC)) {
  default:
    llvm_unreachable("Unknown spill size");
  case 8:
    assert(SM83::GR8RegClass.hasSubClassEq(RC) && "Unknown 1-byte regclass");
    return load ? SM83::LDro : SM83::LDor;
  case 16:
    assert(SM83::GR16RegClass.hasSubClassEq(RC) && "Unknown 2-byte regclass");
    return load ? SM83::LDrro : SM83::LDorr;
  }
}

// whistles in x86 assembly language
static inline const MachineInstrBuilder &
addFrameReference(const MachineInstrBuilder &MIB, int FI) {
  MachineInstr *MI = MIB;
  MachineFunction &MF = *MI->getParent()->getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const MCInstrDesc &MCID = MI->getDesc();
  auto Flags = MachineMemOperand::MONone;
  if (MCID.mayLoad())
    Flags |= MachineMemOperand::MOLoad;
  if (MCID.mayStore())
    Flags |= MachineMemOperand::MOStore;
  MachineMemOperand *MMO =
      MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FI), Flags,
                              MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  return MIB.addFrameIndex(FI).addImm(0).addMemOperand(MMO);
}

void SM83InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DstReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI) const {
  const MachineFunction &MF = *MBB.getParent();
  LLVM_DEBUG(dbgs() << "stack slot size: "
                    << MF.getFrameInfo().getObjectSize(FrameIndex) << '\n');
  unsigned Opc = getLoadStoreRegOpc(RC, TRI, /*load=*/true);
  addFrameReference(BuildMI(MBB, MI, DebugLoc(), get(Opc)).addDef(DstReg),
                    FrameIndex);
}

void SM83InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        Register SrcReg, bool isKill,
                                        int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
  const MachineFunction &MF = *MBB.getParent();
  LLVM_DEBUG(dbgs() << "stack slot size: "
                    << MF.getFrameInfo().getObjectSize(FrameIndex)
                    << " isKill: " << isKill << '\n');
  unsigned Opc = getLoadStoreRegOpc(RC, TRI, /*load=*/false);
  addFrameReference(BuildMI(MBB, MI, DebugLoc(), get(Opc)), FrameIndex)
      .addReg(SrcReg, getKillRegState(isKill));
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

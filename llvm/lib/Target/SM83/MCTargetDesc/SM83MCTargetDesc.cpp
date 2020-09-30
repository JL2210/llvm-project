//===-- SM83MCTargetDesc.cpp - SM83 Target Descriptions -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// This file provides SM83-specific target descriptions.
///
//===----------------------------------------------------------------------===//

#include "SM83MCTargetDesc.h"
#include "SM83MCAsmInfo.h"
#include "TargetInfo/SM83TargetInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "SM83GenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SM83GenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createSM83MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitSM83MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createSM83MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitSM83MCRegisterInfo(X, SM83::A);
  return X;
}

static MCAsmInfo *createSM83MCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT, const MCTargetOptions &MTO) {
  return new SM83MCAsmInfo(TT);
}

extern "C" void LLVMInitializeSM83TargetMC() {
  Target *T = &getTheSM83Target();

  TargetRegistry::RegisterMCAsmInfo(*T, createSM83MCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(*T, createSM83MCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(*T, createSM83MCRegisterInfo);
  TargetRegistry::RegisterMCAsmBackend(*T, createSM83AsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(*T, createSM83MCCodeEmitter);
}

//===-- SM83Subtarget.cpp - SM83 Subtarget Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SM83 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "SM83Subtarget.h"
#include "SM83TargetMachine.h"
#include "GISel/SM83CallLowering.h"
#include "GISel/SM83RegisterBankInfo.h"
#include "SM83.h"
#include "SM83FrameLowering.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "sm83-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SM83GenSubtargetInfo.inc"

SM83Subtarget::SM83Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : SM83GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS), FrameLowering(*this),
      InstrInfo(), RegInfo(), TLInfo(TM, *this) {
  CallLoweringInfo.reset(new SM83CallLowering(*getTargetLowering()));

  auto *RBI = new SM83RegisterBankInfo(*getRegisterInfo());
  RegBankInfo.reset(RBI);
/*
  Legalizer.reset(new SM83LegalizerInfo(*this));

  InstSelector.reset(createSM83InstructionSelector(
    *static_cast<const SM83TargetMachine *>(&TM), *this, *RBI));
*/
}

const CallLowering *SM83Subtarget::getCallLowering() const {
  return CallLoweringInfo.get();
}
const RegisterBankInfo *SM83Subtarget::getRegBankInfo() const {
  return RegBankInfo.get();
}
/*
const LegalizerInfo *SM83Subtarget::getLegalizerInfo() const {
  return Legalizer.get();
}
InstructionSelector *SM83Subtarget::getInstructionSelector() const {
  return InstSelector.get();
}
*/

//===-- SM83Subtarget.cpp - SM83 Subtarget Information --------------------===//
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
#include "GISel/SM83CallLowering.h"
#include "GISel/SM83InstructionSelector.h"
#include "GISel/SM83LegalizerInfo.h"
#include "GISel/SM83RegisterBankInfo.h"
#include "SM83TargetMachine.h"

#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/CodeGen/RegisterBankInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include <string>

using namespace llvm;

#define DEBUG_TYPE "sm83-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SM83GenSubtargetInfo.inc"

SM83Subtarget::SM83Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : SM83GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS), FrameLowering(),
      InstrInfo(*this), RegInfo(TT), TLInfo(TM, *this) {
  CallLoweringInfo.reset(new SM83CallLowering(*getTargetLowering()));

  Legalizer.reset(new SM83LegalizerInfo(TM.createDataLayout()));

  auto *RBI = new SM83RegisterBankInfo(*getRegisterInfo());

  InstSelector.reset(createSM83InstructionSelector(
      *static_cast<const SM83TargetMachine *>(&TM), *this, *RBI));

  RegBankInfo.reset(RBI);
}

const CallLowering *SM83Subtarget::getCallLowering() const {
  return CallLoweringInfo.get();
}
const RegisterBankInfo *SM83Subtarget::getRegBankInfo() const {
  return RegBankInfo.get();
}
const LegalizerInfo *SM83Subtarget::getLegalizerInfo() const {
  return Legalizer.get();
}
InstructionSelector *SM83Subtarget::getInstructionSelector() const {
  return InstSelector.get();
}

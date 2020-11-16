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
#include "SM83.h"
#include "SM83FrameLowering.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "sm83-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SM83GenSubtargetInfo.inc"

void SM83Subtarget::anchor() {}

SM83Subtarget::SM83Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : SM83GenSubtargetInfo(TT, CPU, FS), FrameLowering(*this),
      InstrInfo(), RegInfo(), TLInfo(TM, *this) {}

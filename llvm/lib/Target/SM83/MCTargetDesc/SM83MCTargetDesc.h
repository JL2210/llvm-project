//===-- SM83MCTargetDesc.h - SM83 Target Descriptions ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides SM83 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83MCTARGETDESC_H
#define LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83MCTARGETDESC_H

#include "llvm/Config/config.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class Target;

MCCodeEmitter *createSM83MCCodeEmitter(const MCInstrInfo &MCII,
                                       const MCRegisterInfo &MRI,
                                       MCContext &Ctx);

MCAsmBackend *createSM83AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter>
createSM83ELFObjectWriter();
}

// Defines symbolic names for SM83 registers.
#define GET_REGINFO_ENUM
#include "SM83GenRegisterInfo.inc"

// Defines symbolic names for SM83 instructions.
#define GET_INSTRINFO_ENUM
#include "SM83GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SM83GenSubtargetInfo.inc"

#endif

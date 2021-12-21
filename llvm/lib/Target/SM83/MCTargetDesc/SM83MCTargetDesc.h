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

#include <cstdint>

// Defines symbolic names for SM83 registers.
#define GET_REGINFO_ENUM
#include "SM83GenRegisterInfo.inc"

// Defines symbolic names for SM83 instructions.
#define GET_INSTRINFO_ENUM
#include "SM83GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SM83GenSubtargetInfo.inc"

#endif

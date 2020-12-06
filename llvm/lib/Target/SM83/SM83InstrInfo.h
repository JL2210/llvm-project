//===-- SM83InstrInfo.h - SM83 Instruction Information ----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SM83_SM83INSTRINFO_H
#define LLVM_LIB_TARGET_SM83_SM83INSTRINFO_H

#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "SM83GenInstrInfo.inc"

namespace llvm {

class SM83InstrInfo : public SM83GenInstrInfo {
public:
  SM83InstrInfo();
};

} // namespace llvm

#endif

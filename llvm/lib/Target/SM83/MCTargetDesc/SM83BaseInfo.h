//===-- SM83BaseInfo.h - Top-level definitions for SM83 MC --------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone enum definitions for the SM83 target
// useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83BASEINFO_H
#define LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83BASEINFO_H

#include "llvm/MC/MCInstrDesc.h"

namespace llvm {
namespace SM83 {

enum OperandType : unsigned {
  OPERAND_CONDITION = MCOI::OPERAND_FIRST_TARGET,
}

enum Condition {
  COND_NZ = 0,
  COND_Z = 1,
  COND_NC = 2,
  COND_C = 3,
  LAST_VALID_COND = COND_C,
  COND_INVALID
};

}
} // namespace llvm

#endif

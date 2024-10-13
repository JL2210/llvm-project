//===-- SM83ISelLowering.h - SM83 Selection Lowering ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H
#define LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/IR/CallingConv.h"

namespace llvm {

class TargetMachine;
class SM83Subtarget;

class SM83TargetLowering : public TargetLowering {
public:
  explicit SM83TargetLowering(const TargetMachine &TM,
                              const SM83Subtarget &STI);

  /// Selects the correct CCAssignFn for a given CallingConvention value.
  CCAssignFn *CCAssignFnForCall(CallingConv::ID CC, bool IsVarArg) const;
  CCAssignFn *CCAssignFnForReturn(CallingConv::ID CC) const;

  bool supportSwiftError() const override { return false; }
};

} // namespace llvm

#endif

//===-- SM83TargetInfo.cpp - SM83 Target Implementation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SM83TargetInfo.h"

#include "llvm/Support/TargetRegistry.h"

namespace llvm {

Target &getTheSM83Target() {
  static Target TheSM83Target;
  return TheSM83Target;
}

}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSM83TargetInfo() {
  llvm::RegisterTarget<llvm::Triple::sm83> X(llvm::getTheSM83Target(), "sm83",
                                            "Sharp SM83 Microcomputer", "SM83");
}

// FIXME: Temporary stub - this function must be defined for linking
// to succeed and will be called unconditionally by llc, so must be a no-op.
// Remove once this function is properly implemented.
extern "C" void LLVMInitializeSM83TargetMC() {}

//===-- SM83TargetInfo.cpp - SM83 Target Implementation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/SM83TargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

namespace llvm {

Target &getTheSM83Target() {
  static Target TheSM83Target;
  return TheSM83Target;
}

}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSM83TargetInfo() {
  llvm::RegisterTarget<llvm::Triple::sm83> X(llvm::getTheSM83Target(), "SM83",
                                            "Sharp SM83 Microcomputer", "SM83");
}


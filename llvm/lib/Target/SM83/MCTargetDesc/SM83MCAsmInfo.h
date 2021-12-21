//===-- SM83MCAsmInfo.h - SM83 Asm Info ------------------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the SM83MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83MCASMINFO_H
#define LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83MCASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
class Triple;
class StringRef;

class SM83MCAsmInfo : public MCAsmInfo {
  void anchor();

public:
  explicit SM83MCAsmInfo(const Triple &TargetTriple,
                         const MCTargetOptions &Options);
};

} // namespace llvm

#endif

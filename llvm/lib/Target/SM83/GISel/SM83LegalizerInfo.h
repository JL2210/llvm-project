//===- SM83LegalizerInfo.h ---------------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This file declares the targeting of the LegalizerInfo class for SM83.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_GISEL_SM83LEGALIZERINFO_H
#define LLVM_LIB_TARGET_SM83_GISEL_SM83LEGALIZERINFO_H

#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/IR/DataLayout.h"

namespace llvm {

class SM83LegalizerInfo : public LegalizerInfo {
public:
  SM83LegalizerInfo(const DataLayout DL);
};

} // namespace llvm
#endif

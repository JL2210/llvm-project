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

namespace llvm {
namespace SM83Condition {
enum Condition {
  NZ = 0,
  Z  = 1,
  NC = 2,
  C  = 3,
};
}
}

#endif

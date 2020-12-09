//===- SM83LegalizerInfo.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This file implements the targeting of the LegalizerInfo class for SM83.
//===----------------------------------------------------------------------===//

#include "SM83LegalizerInfo.h"

using namespace llvm;

SM83LegalizerInfo::SM83LegalizerInfo(const DataLayout DL)
  : LegalizerInfo() {
  using namespace TargetOpcode;
  const LLT p0 = LLT::pointer(0, DL.getPointerSizeInBits(0));
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);

  getActionDefinitionsBuilder({G_IMPLICIT_DEF, G_CONSTANT})
    .legalFor({p0, s1, s8, s16, s32});

  getActionDefinitionsBuilder(G_PHI)
    .legalFor({s8, s16});

  getActionDefinitionsBuilder(G_ADD)
    .legalFor({s8, s16});
  getActionDefinitionsBuilder({G_SUB, G_AND, G_XOR, G_OR})
    .legalFor({s8});

  getActionDefinitionsBuilder({G_FRAME_INDEX, G_BLOCK_ADDR})
    .legalFor({p0});

  computeTables();
}

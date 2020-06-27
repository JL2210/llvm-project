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

SM83LegalizerInfo::SM83LegalizerInfo(const DataLayout DL) : LegalizerInfo() {
  using namespace TargetOpcode;
  const LLT p0 = LLT::pointer(0, DL.getPointerSizeInBits(0));
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);

  getActionDefinitionsBuilder({G_IMPLICIT_DEF, G_CONSTANT})
      .legalFor({p0, s1, s8, s16});

  getActionDefinitionsBuilder(G_GLOBAL_VALUE).legalFor({p0});

  getActionDefinitionsBuilder(G_PHI).legalFor({s8, s16});

  getActionDefinitionsBuilder({G_ADD, G_XOR}).legalFor({s8, s16});
  getActionDefinitionsBuilder({G_SUB, G_AND, G_OR})
      .legalFor({s8})
      .maxScalar(0, s8);

  getActionDefinitionsBuilder(G_SEXT).legalForCartesianProduct({s8}, {s1});

  getActionDefinitionsBuilder(G_ANYEXT)
      .widenScalarToNextPow2(0)
      .clampScalar(0, s8, s16)
      .maxScalar(1, s16);

  getActionDefinitionsBuilder(G_ICMP).legalForCartesianProduct({s1},
                                                               {p0, s8, s16});

  getActionDefinitionsBuilder(G_UNMERGE_VALUES)
      .legalForCartesianProduct({s8}, {s16});

  getActionDefinitionsBuilder(G_MERGE_VALUES)
      .legalForCartesianProduct({s16}, {s8});

  getActionDefinitionsBuilder(G_INTTOPTR).legalForCartesianProduct({p0}, {s16});

  getActionDefinitionsBuilder(G_PTRTOINT).legalForCartesianProduct({s16}, {p0});

  getActionDefinitionsBuilder({G_LOAD, G_STORE})
      .legalForCartesianProduct({s8}, {p0});

  getActionDefinitionsBuilder(G_PTR_ADD).legalForCartesianProduct({p0}, {s16});

  getActionDefinitionsBuilder({G_FRAME_INDEX, G_BLOCK_ADDR}).legalFor({p0});

  getActionDefinitionsBuilder(G_BRINDIRECT).legalFor({p0});

  getLegacyLegalizerInfo().computeTables();
}

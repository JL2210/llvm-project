//===- SM83LegalizerInfo.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This file implements the targeting of the LegalizerInfo class for SM83.
//===----------------------------------------------------------------------===//

#include "SM83ISelLowering.h"
#include "SM83LegalizerInfo.h"
#include "SM83Subtarget.h"
#include "SM83TargetMachine.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/LowLevelType.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/IR/DataLayout.h"

using namespace llvm;

SM83LegalizerInfo::SM83LegalizerInfo(const SM83Subtarget &ST) : LegalizerInfo() {
  using namespace TargetOpcode;
  const LLT p0 = LLT::pointer(0, 16);
  const LLT p1 = LLT::pointer(1, 8);
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);

  getActionDefinitionsBuilder({G_IMPLICIT_DEF, G_CONSTANT})
      .legalFor({p0, p1, s1, s8, s16});

  getActionDefinitionsBuilder(G_GLOBAL_VALUE)
      .legalFor({p0, p1});

  getActionDefinitionsBuilder(G_PHI)
      .legalFor({s8, s16, p0, p1});

  getActionDefinitionsBuilder({G_ADD, G_XOR, G_SUB, G_AND, G_OR})
      .legalFor({s8})
      .maxScalar(0, s8);

  getActionDefinitionsBuilder({G_UADDO, G_USUBO})
      .legalFor({s8, s1})
      .maxScalar(0, s8)
      .maxScalar(1, s1)
      .maxScalar(2, s8)
      .maxScalar(3, s8);

  getActionDefinitionsBuilder({G_UADDE, G_USUBE})
      .legalFor({s8, s1})
      .maxScalar(0, s8)
      .maxScalar(1, s1)
      .maxScalar(2, s8)
      .maxScalar(3, s8)
      .maxScalar(4, s1);

  getActionDefinitionsBuilder(G_ABS)
      .lower();

  getActionDefinitionsBuilder({G_SHL, G_ASHR, G_LSHR})
      .legalFor({{s8, s8}})
      .maxScalar(0, s8)
      .maxScalar(1, s8)
      .lower();

  getActionDefinitionsBuilder(G_SEXT)
      .legalFor({{s8, s1}});

  getActionDefinitionsBuilder(G_ANYEXT)
      .widenScalarToNextPow2(0)
      .clampScalar(0, s8, s16)
      .maxScalar(1, s16);

  getActionDefinitionsBuilder(G_ICMP)
      .legalForCartesianProduct({s1},
                                {p0, p1, s8, s16});

  getActionDefinitionsBuilder(G_UNMERGE_VALUES)
      .legalFor({{s8, s16}});
  getActionDefinitionsBuilder(G_MERGE_VALUES)
      .legalFor({{s16, s8}});

  getActionDefinitionsBuilder(G_INTTOPTR)
      .legalFor({{p0, s16}})
      .legalFor({{p1, s8}});
  getActionDefinitionsBuilder(G_PTRTOINT)
      .legalForCartesianProduct({s16}, {p0, p1})
      .legalFor({{s8, p1}});

  // this works for both:
  // %0:_(s8) = G_LOAD %1:_(p0)
  // G_STORE %0:_(s8), %1:_(p0)
  getActionDefinitionsBuilder({G_LOAD, G_STORE})
      .legalForCartesianProduct({s8}, {p0, p1})
      .maxScalar(0, s8);

  getActionDefinitionsBuilder(G_PTR_ADD)
      .legalForCartesianProduct({p0}, {s8, s16})
      .legalFor({{p1, s8}});

  getActionDefinitionsBuilder({G_FRAME_INDEX, G_BLOCK_ADDR})
      .legalFor({p0});

  getActionDefinitionsBuilder(G_BRCOND)
      .legalFor({s1});
  getActionDefinitionsBuilder(G_BRINDIRECT)
      .legalFor({p0});

  getLegacyLegalizerInfo().computeTables();
  verify(*ST.getInstrInfo());
}

// nabbed from z80 backend by jacobly0
bool SM83LegalizerInfo::legalizeCustom(
    LegalizerHelper &Helper, MachineInstr &MI,
    LostDebugLocObserver &LocObserver) const {
  Helper.MIRBuilder.setInstrAndDebugLoc(MI);
  switch (MI.getOpcode()) {
  default:
    // No idea what to do.
    return false;
  }
}

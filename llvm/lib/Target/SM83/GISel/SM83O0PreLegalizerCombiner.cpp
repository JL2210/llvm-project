//=== lib/CodeGen/GlobalISel/SM83O0PreLegalizerCombiner.cpp ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass does combining of machine instructions at the generic MI level,
// before the legalizer.
//
//===----------------------------------------------------------------------===//

#include "GISel/SM83CombinerPasses.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"

#define DEBUG_TYPE "SM83-O0-prelegalizer-combiner"

using namespace llvm;

class SM83O0PreLegalizerCombinerHelperState {
protected:
  CombinerHelper &Helper;

public:
  SM83O0PreLegalizerCombinerHelperState(CombinerHelper &Helper)
      : Helper(Helper) {}
};

#define SM83O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS
#include "SM83GenO0PreLegalizeGICombiner.inc"
#undef SM83O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS

namespace {
#define SM83O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H
#include "SM83GenO0PreLegalizeGICombiner.inc"
#undef SM83O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H

class SM83O0PreLegalizerCombinerInfo : public CombinerInfo {
  GISelKnownBits *KB;
  MachineDominatorTree *MDT;
  SM83GenO0PreLegalizerCombinerHelperRuleConfig GeneratedRuleCfg;

public:
  SM83O0PreLegalizerCombinerInfo(bool EnableOpt, bool OptSize, bool MinSize,
                                 GISelKnownBits *KB, MachineDominatorTree *MDT)
      : CombinerInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, EnableOpt, OptSize, MinSize),
        KB(KB), MDT(MDT) {
    assert(!EnableOpt && !OptSize && !MinSize &&
           "-O0 Combiner pass should only be run with optimizations disabled");
    if (!GeneratedRuleCfg.parseCommandLineOption())
      report_fatal_error("Invalid rule identifier");
  }

  virtual bool combine(GISelChangeObserver &Observer, MachineInstr &MI,
                       MachineIRBuilder &B) const override;
};

bool SM83O0PreLegalizerCombinerInfo::combine(GISelChangeObserver &Observer,
                                             MachineInstr &MI,
                                             MachineIRBuilder &B) const {
  CombinerHelper Helper(Observer, B, KB, MDT);
  SM83GenO0PreLegalizerCombinerHelper Generated(GeneratedRuleCfg, Helper);

  if (Generated.tryCombineAll(Observer, MI, B))
    return true;

  return false;
}

#define SM83O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP
#include "SM83GenO0PreLegalizeGICombiner.inc"
#undef SM83O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP

// Pass boilerplate
// ================

class SM83O0PreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  SM83O0PreLegalizerCombiner();

  StringRef getPassName() const override {
    return "SM83O0PreLegalizerCombiner";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // end anonymous namespace

void SM83O0PreLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  AU.addRequired<GISelKnownBitsAnalysis>();
  AU.addPreserved<GISelKnownBitsAnalysis>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

SM83O0PreLegalizerCombiner::SM83O0PreLegalizerCombiner()
    : MachineFunctionPass(ID) {
  initializeSM83O0PreLegalizerCombinerPass(*PassRegistry::getPassRegistry());
}

bool SM83O0PreLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel))
    return false;
  auto &TPC = getAnalysis<TargetPassConfig>();

  const Function &F = MF.getFunction();
  bool EnableOpt =
      MF.getTarget().getOptLevel() != CodeGenOpt::None && !skipFunction(F);
  GISelKnownBits *KB = &getAnalysis<GISelKnownBitsAnalysis>().get(MF);
  SM83O0PreLegalizerCombinerInfo PCInfo(EnableOpt, F.hasOptSize(),
                                        F.hasMinSize(), KB, nullptr /* MDT */);
  Combiner C(PCInfo, &TPC);
  return C.combineMachineInstrs(MF, nullptr /* CSEInfo */);
}

char SM83O0PreLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(SM83O0PreLegalizerCombiner, DEBUG_TYPE,
                      "Combine SM83 machine instrs before legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelKnownBitsAnalysis)
INITIALIZE_PASS_DEPENDENCY(GISelCSEAnalysisWrapperPass)
INITIALIZE_PASS_END(SM83O0PreLegalizerCombiner, DEBUG_TYPE,
                    "Combine SM83 machine instrs before legalization", false,
                    false)

namespace llvm {
FunctionPass *createSM83O0PreLegalizerCombiner() {
  return new SM83O0PreLegalizerCombiner();
}
} // end namespace llvm

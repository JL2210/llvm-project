//=== lib/CodeGen/GlobalISel/SM83Combiner.cpp -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass does combining of machine instructions at the generic MI level,
// before and after the legalizer.
//
// Therefore, this pass must not affect the legality of instructions.
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
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "sm83-combiner"

using namespace llvm;

class SM83CombinerHelperState {
protected:
  CombinerHelper &Helper;

public:
  SM83CombinerHelperState(CombinerHelper &Helper)
      : Helper(Helper) {}
};

#define SM83COMBINERHELPER_GENCOMBINERHELPER_DEPS
#include "SM83GenGICombiner.inc"
#undef SM83COMBINERHELPER_GENCOMBINERHELPER_DEPS

namespace {
#define SM83COMBINERHELPER_GENCOMBINERHELPER_H
#include "SM83GenGICombiner.inc"
#undef SM83COMBINERHELPER_GENCOMBINERHELPER_H

class SM83CombinerInfo : public CombinerInfo {
  GISelKnownBits *KB;
  MachineDominatorTree *MDT;
  SM83GenCombinerHelperRuleConfig GeneratedRuleCfg;

public:
  SM83CombinerInfo(bool EnableOpt, bool OptSize, bool MinSize,
                   GISelKnownBits *KB, MachineDominatorTree *MDT)
      : CombinerInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, EnableOpt, OptSize, MinSize),
        KB(KB), MDT(MDT) {
    assert(EnableOpt == true &&
           "Combiner pass should only be run with optimizations enabled");
    if (!GeneratedRuleCfg.parseCommandLineOption())
      report_fatal_error("Invalid rule identifier");
  }

  virtual bool combine(GISelChangeObserver &Observer, MachineInstr &MI,
                       MachineIRBuilder &B) const override;
};

bool SM83CombinerInfo::combine(GISelChangeObserver &Observer, MachineInstr &MI,
                               MachineIRBuilder &B) const {
  CombinerHelper Helper(Observer, B, KB, MDT);
  SM83GenCombinerHelper Generated(GeneratedRuleCfg, Helper);

  if (Generated.tryCombineAll(Observer, MI, B))
    return true;

  return false;
}

#define SM83COMBINERHELPER_GENCOMBINERHELPER_CPP
#include "SM83GenGICombiner.inc"
#undef SM83COMBINERHELPER_GENCOMBINERHELPER_CPP

// Pass boilerplate
// ================

class SM83Combiner : public MachineFunctionPass {
public:
  static char ID;

  SM83Combiner();

  StringRef getPassName() const override { return "SM83Combiner"; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // end anonymous namespace

void SM83Combiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  AU.addRequired<GISelKnownBitsAnalysis>();
  AU.addPreserved<GISelKnownBitsAnalysis>();
  AU.addRequired<MachineDominatorTree>();
  AU.addPreserved<MachineDominatorTree>();
  AU.addRequired<GISelCSEAnalysisWrapperPass>();
  AU.addPreserved<GISelCSEAnalysisWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

SM83Combiner::SM83Combiner()
    : MachineFunctionPass(ID) {
  initializeSM83CombinerPass(*PassRegistry::getPassRegistry());
}

bool SM83Combiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel))
    return false;
  auto &TPC = getAnalysis<TargetPassConfig>();

  // Enable CSE.
  GISelCSEAnalysisWrapper &Wrapper =
      getAnalysis<GISelCSEAnalysisWrapperPass>().getCSEWrapper();
  auto *CSEInfo = &Wrapper.get(TPC.getCSEConfig());

  const Function &F = MF.getFunction();
  bool EnableOpt =
      MF.getTarget().getOptLevel() != CodeGenOpt::None && !skipFunction(F);
  GISelKnownBits *KB = &getAnalysis<GISelKnownBitsAnalysis>().get(MF);
  MachineDominatorTree *MDT = &getAnalysis<MachineDominatorTree>();
  SM83CombinerInfo PCInfo(EnableOpt, F.hasOptSize(),
                                         F.hasMinSize(), KB, MDT);
  Combiner C(PCInfo, &TPC);
  return C.combineMachineInstrs(MF, CSEInfo);
}

char SM83Combiner::ID = 0;
INITIALIZE_PASS_BEGIN(SM83Combiner, DEBUG_TYPE,
                      "Combine SM83 machine instrs",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelKnownBitsAnalysis)
INITIALIZE_PASS_DEPENDENCY(GISelCSEAnalysisWrapperPass)
INITIALIZE_PASS_END(SM83Combiner, DEBUG_TYPE,
                    "Combine SM83 machine instrs", false,
                    false)

namespace llvm {
FunctionPass *createSM83Combiner() {
  return new SM83Combiner();
}
} // end namespace llvm

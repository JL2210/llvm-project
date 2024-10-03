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

#include "SM83Subtarget.h"
#include "GISel/SM83CombinerPasses.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutor.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"

#define GET_GICOMBINER_DEPS
#include "SM83GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "SM83-O0-prelegalizer-combiner"

using namespace llvm;

namespace {
#define GET_GICOMBINER_TYPES
#include "SM83GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

class SM83O0PreLegalizerCombinerImpl : public GIMatchTableExecutor {
protected:
  CombinerHelper &Helper;
  const SM83O0PreLegalizerCombinerImplRuleConfig &RuleConfig;

  const SM83Subtarget &STI;
  GISelChangeObserver &Observer;
  MachineIRBuilder &B;
  MachineFunction &MF;

  MachineRegisterInfo &MRI;


public:
  SM83O0PreLegalizerCombinerImpl(
      const SM83O0PreLegalizerCombinerImplRuleConfig &RuleConfig,
      GISelChangeObserver &Observer, MachineIRBuilder &B,
      CombinerHelper &Helper);

  static const char *getName() { return "SM83O0PreLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "SM83GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "SM83GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

SM83O0PreLegalizerCombinerImpl::SM83O0PreLegalizerCombinerImpl(
    const SM83O0PreLegalizerCombinerImplRuleConfig &RuleConfig,
    GISelChangeObserver &Observer, MachineIRBuilder &B,
    CombinerHelper &Helper)
    : Helper(Helper), RuleConfig(RuleConfig),
      STI(B.getMF().getSubtarget<SM83Subtarget>()), Observer(Observer), B(B),
      MF(B.getMF()), MRI(*B.getMRI()),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "SM83GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

class SM83O0PreLegalizerCombinerInfo : public CombinerInfo {
  GISelKnownBits *KB;
  MachineDominatorTree *MDT;
  SM83O0PreLegalizerCombinerImplRuleConfig RuleConfig;

public:
  SM83O0PreLegalizerCombinerInfo(bool EnableOpt, bool OptSize, bool MinSize,
                                 GISelKnownBits *KB, MachineDominatorTree *MDT)
      : CombinerInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, EnableOpt, OptSize, MinSize),
        KB(KB), MDT(MDT) {
    assert(!EnableOpt && !OptSize && !MinSize &&
           "-O0 Combiner pass should only be run with optimizations disabled");
    if (!RuleConfig.parseCommandLineOption())
      report_fatal_error("Invalid rule identifier");
  }

  virtual bool combine(GISelChangeObserver &Observer, MachineInstr &MI,
                       MachineIRBuilder &B) const override;
};

bool SM83O0PreLegalizerCombinerInfo::combine(GISelChangeObserver &Observer,
                                             MachineInstr &MI,
                                             MachineIRBuilder &B) const {
  CombinerHelper Helper(Observer, B, /*IsPreLegalize=*/true, KB, MDT);
  SM83O0PreLegalizerCombinerImpl Impl(RuleConfig, Observer, B, Helper);
  Impl.setupMF(*MI.getMF(), KB);

  if (Impl.tryCombineAll(MI))
    return true;

  return false;
}

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

//=== lib/CodeGen/GlobalISel/SM83PreLegalizerCombiner.cpp -----------------------------===//
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

#include "SM83Subtarget.h"
#include "GISel/SM83CombinerPasses.h"
#include "llvm/CodeGen/GlobalISel/CSEInfo.h"
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
#include "SM83GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "sm83-prelegalizer-combiner"

using namespace llvm;

namespace {

#define GET_GICOMBINER_TYPES
#include "SM83GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

class SM83PreLegalizerCombinerImpl : public GIMatchTableExecutor {
protected:
  CombinerHelper &Helper;
  const SM83PreLegalizerCombinerImplRuleConfig &RuleConfig;

  const SM83Subtarget &STI;
  GISelChangeObserver &Observer;
  MachineIRBuilder &B;
  MachineFunction &MF;

  MachineRegisterInfo &MRI;

public:
  SM83PreLegalizerCombinerImpl(
      const SM83PreLegalizerCombinerImplRuleConfig &RuleConfig,
      GISelChangeObserver &Observer, MachineIRBuilder &B,
      CombinerHelper &Helper);

  static const char *getName() { return "SM83PreLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "SM83GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "SM83GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

SM83PreLegalizerCombinerImpl::SM83PreLegalizerCombinerImpl(
    const SM83PreLegalizerCombinerImplRuleConfig &RuleConfig,
    GISelChangeObserver &Observer,
    MachineIRBuilder &B, CombinerHelper &Helper)
    : Helper(Helper), RuleConfig(RuleConfig),
      STI(B.getMF().getSubtarget<SM83Subtarget>()), Observer(Observer), B(B),
      MF(B.getMF()), MRI(*B.getMRI()),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "SM83GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

class SM83PreLegalizerCombinerInfo : public CombinerInfo {
  GISelKnownBits *KB;
  MachineDominatorTree *MDT;
  SM83PreLegalizerCombinerImplRuleConfig RuleConfig;

public:
  SM83PreLegalizerCombinerInfo(bool EnableOpt, bool OptSize, bool MinSize,
                   GISelKnownBits *KB, MachineDominatorTree *MDT)
      : CombinerInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, EnableOpt, OptSize, MinSize),
        KB(KB), MDT(MDT) {
    assert(EnableOpt == true &&
           "Combiner pass should only be run with optimizations enabled");
    if (!RuleConfig.parseCommandLineOption())
      report_fatal_error("Invalid rule identifier");
  }

  virtual bool combine(GISelChangeObserver &Observer, MachineInstr &MI,
                       MachineIRBuilder &B) const override;
};

bool SM83PreLegalizerCombinerInfo::combine(GISelChangeObserver &Observer, MachineInstr &MI,
                               MachineIRBuilder &B) const {
  CombinerHelper Helper(Observer, B, /*IsPreLegalize=*/true, KB, MDT);
  SM83PreLegalizerCombinerImpl Impl(RuleConfig, Observer, B, Helper);
  Impl.setupMF(*MI.getMF(), KB);

  if (Impl.tryCombineAll(MI))
    return true;

  return false;
}

// Pass boilerplate
// ================

class SM83PreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  SM83PreLegalizerCombiner();

  StringRef getPassName() const override {
    return "SM83PreLegalizerCombiner";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // end anonymous namespace

void SM83PreLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
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

SM83PreLegalizerCombiner::SM83PreLegalizerCombiner() : MachineFunctionPass(ID) {
  initializeSM83PreLegalizerCombinerPass(*PassRegistry::getPassRegistry());
}

bool SM83PreLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
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
  SM83PreLegalizerCombinerInfo PCInfo(EnableOpt, F.hasOptSize(), F.hasMinSize(), KB, MDT);
  Combiner C(PCInfo, &TPC);
  return C.combineMachineInstrs(MF, CSEInfo);
}

char SM83PreLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(SM83PreLegalizerCombiner, DEBUG_TYPE, "Combine SM83 machine instrs before legalization",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelKnownBitsAnalysis)
INITIALIZE_PASS_DEPENDENCY(GISelCSEAnalysisWrapperPass)
INITIALIZE_PASS_END(SM83PreLegalizerCombiner, DEBUG_TYPE, "Combine SM83 machine instrs before legalization",
                    false, false)

namespace llvm {
FunctionPass *createSM83PreLegalizerCombiner() { return new SM83PreLegalizerCombiner(); }
} // end namespace llvm

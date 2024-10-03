//===-- SM83TargetMachine.cpp - Define TargetMachine for SM83 -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the SM83-specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#include "SM83TargetMachine.h"
#include "GISel/SM83CombinerPasses.h"
#include "TargetInfo/SM83TargetInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/GlobalISel/CSEInfo.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/LoadStoreOpt.h"
#include "llvm/CodeGen/GlobalISel/Localizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/InitializePasses.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetOptions.h"

namespace llvm {

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSM83Target() {
  RegisterTargetMachine<SM83TargetMachine> X(getTheSM83Target());
  auto PR = PassRegistry::getPassRegistry();
  initializeGlobalISel(*PR);
  initializeSM83O0PreLegalizerCombinerPass(*PR);
  initializeSM83PreLegalizerCombinerPass(*PR);
}

static const char SM83DataLayout[] = "e-p:16:8-i16:8-i32:8-i64:8-a:0:8-n8:16";

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.has_value() ? *RM : Reloc::Static;
}

SM83TargetMachine::SM83TargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, SM83DataLayout, TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileRGB9>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();

  setGlobalISel(true);
  setGlobalISelAbort(GlobalISelAbortMode::Enable);
}

class SM83PassConfig : public TargetPassConfig {
public:
  SM83PassConfig(SM83TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  SM83TargetMachine &getSM83TargetMachine() const {
    return getTM<SM83TargetMachine>();
  }

  bool addIRTranslator() override;
  void addPreLegalizeMachineIR() override;
  bool addLegalizeMachineIR() override;
  void addPreRegBankSelect() override;
  bool addRegBankSelect() override;
  void addPreGlobalInstructionSelect() override;
  bool addGlobalInstructionSelect() override;
  void addPreRegAlloc() override;
  void addFastRegAlloc() override;

  std::unique_ptr<CSEConfigBase> getCSEConfig() const override;
};

bool SM83PassConfig::addIRTranslator() {
  addPass(new IRTranslator(getOptLevel()));
  return false;
}

void SM83PassConfig::addPreLegalizeMachineIR() {
  if (getOptLevel() == CodeGenOpt::None) {
    addPass(createSM83O0PreLegalizerCombiner());
  } else {
    addPass(createSM83PreLegalizerCombiner());
    addPass(new LoadStoreOpt());
  }
}

bool SM83PassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

void SM83PassConfig::addPreRegBankSelect() {
  if (getOptLevel() != CodeGenOpt::None) {
    addPass(new LoadStoreOpt());
  }
}

bool SM83PassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}
void SM83PassConfig::addPreGlobalInstructionSelect() {
  addPass(new Localizer());
}
bool SM83PassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect());
  return false;
}

void SM83PassConfig::addPreRegAlloc() {
  if (TM->getOptLevel() != CodeGenOpt::None) {
    addPass(&PeepholeOptimizerID);
  }
}

void SM83PassConfig::addFastRegAlloc() {
  // FastRegAlloc can't handle the register pressure
  if (usingDefaultRegAlloc())
    addOptimizedRegAlloc();
  else
    TargetPassConfig::addFastRegAlloc();
}

TargetPassConfig *SM83TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SM83PassConfig(*this, PM);
}

std::unique_ptr<CSEConfigBase> SM83PassConfig::getCSEConfig() const {
  return getStandardCSEConfigForOpt(TM->getOptLevel());
}

} // end namespace llvm

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

#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/InitializePasses.h"

#include "TargetInfo/SM83TargetInfo.h"

namespace llvm {

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSM83Target() {
  RegisterTargetMachine<SM83TargetMachine> X(getTheSM83Target());
  auto PR = PassRegistry::getPassRegistry();
  initializeGlobalISel(*PR);
}

static const char SM83DataLayout[] = "e-p:16:8-i16:8-i32:8-i64:8-a:0:8-m:e-n8:16";

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  return RM.hasValue() ? *RM : Reloc::Static;
}

SM83TargetMachine::SM83TargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     Optional<Reloc::Model> RM,
                                     Optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, SM83DataLayout, TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
}

class SM83PassConfig : public TargetPassConfig {
public:
  SM83PassConfig(SM83TargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  SM83TargetMachine &getSM83TargetMachine() const {
    return getTM<SM83TargetMachine>();
  }

  bool addIRTranslator() override;
  bool addLegalizeMachineIR() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;
};

bool SM83PassConfig::addIRTranslator() {
  return false;
}
bool SM83PassConfig::addLegalizeMachineIR() {
  return false;
}
bool SM83PassConfig::addRegBankSelect() {
  return false;
}
bool SM83PassConfig::addGlobalInstructionSelect() {
  return false;
}

TargetPassConfig *SM83TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SM83PassConfig(*this, PM);
}

} // end namespace llvm

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

#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"

namespace llvm {

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSM83Target() {
  RegisterTargetMachine<SM83TargetMachine> X(getTheSM83Target());
}

static const char SM83DataLayout[] = "e-p:16:8-i8:8-i16:8-n16-S8";

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
      TLOF(make_unique<SM83TargetObjectFile>()) {
  initAsmInfo();
}

TargetPassConfig *SM83TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TargetPassConfig(*this, PM);
}

} // end namespace llvm

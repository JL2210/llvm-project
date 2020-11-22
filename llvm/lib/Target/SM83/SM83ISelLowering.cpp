//===-- SM83ISelLowering.cpp - SM83 Instruction Selection Lowering --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SM83TargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "SM83ISelLowering.h"
#include "SM83.h"
#include "SM83RegisterInfo.h"
#include "SM83Subtarget.h"
#include "SM83TargetMachine.h"

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "sm83-lower"

SM83TargetLowering::SM83TargetLowering(const TargetMachine &TM,
                                       const SM83Subtarget &STI)
    : TargetLowering(TM) {
  // Set up the register classes.
  addRegisterClass(MVT::i8, &SM83::GR8RegClass);
  addRegisterClass(MVT::i16, &SM83::GR16RegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(SM83::SP);

  // TODO: add all necessary setOperationAction calls.

  setBooleanContents(ZeroOrOneBooleanContent);
}

#include "SM83GenCallingConv.inc"

/// Selects the correct CCAssignFn for a given CallingConvention value.
CCAssignFn *SM83TargetLowering::CCAssignFnForCall(CallingConv::ID CC,
                                                  bool IsVarArg) const {
  switch (CC) {
  default:
    report_fatal_error("Unsupported calling convention.");
  case CallingConv::C:
  case CallingConv::Fast:
    return CC_SM83;
  }
}

CCAssignFn *
SM83TargetLowering::CCAssignFnForReturn(CallingConv::ID CC) const {
  return RetCC_SM83;
}

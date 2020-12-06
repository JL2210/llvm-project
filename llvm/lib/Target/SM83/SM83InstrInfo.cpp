//===-- SM83InstrInfo.cpp - SM83 Instruction Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SM83 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "SM83InstrInfo.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "SM83GenInstrInfo.inc"

using namespace llvm;

SM83InstrInfo::SM83InstrInfo() : SM83GenInstrInfo() {}

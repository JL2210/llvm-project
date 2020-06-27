//===-- SM83.td - Target definition file for the Sharp SM83 --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------------===//
//
// This is a target description file for the Sharp SM83 architecture, referred
// to here as the "SM83" architecture.
//
//===------------------------------------------------------------------------===//

// Get the target-independent interfaces which we are implementing...
//
include "llvm/Target/Target.td"

// Target-dependent interfaces
//
include "SM83RegisterInfo.td"
include "SM83InstrInfo.td"
include "SM83CallingConv.td"

def SM83InstrInfo : InstrInfo;

def SM83 final : public Target {
	let InstructionSet = SM83InstrInfo;
}
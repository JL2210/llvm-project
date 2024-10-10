//===-- SM83CallingConv.td - Calling Convention for SM83 ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the calling convention for the Sharp SM83 architecture.
//
//===----------------------------------------------------------------------===//

def CC_SM83_VarArg : CallingConv <[
  CCAssignToStack<0, 1>,
]>;

def CC_SM83 : CallingConv <[
  CCIfVarArg<CCDelegateTo<CC_SM83_VarArg>>,
  CCIfType<[i1], CCPromoteToType<i8>>,
  CCIfType<[i8], CCAssignToReg<[A, E]>>,
  // TODO: this doesn't exactly match SDCC's calling convention.
  //       Probably will need to be written by hand.
  CCIfType<[i16], CCAssignToReg<[BC, DE]>>,
  CCAssignToStack<0, 1>,
]>;

def RetCC_SM83 : CallingConv <[
  CCIfType<[i1], CCPromoteToType<i8>>,
  CCIfType<[i8], CCAssignToReg<[A]>>,
  CCIfType<[i16], CCAssignToReg<[BC, DE]>>,
]>;

def CC_SM83_Interrupt : CallingConv <[
]>;

def CSR_NoRegs : CalleeSavedRegs<(add)>;
def CSR : CalleeSavedRegs<(add BC)>;
def CSR_Interrupt : CalleeSavedRegs<(add AF, BC, DE, HL)>;

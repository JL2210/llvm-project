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

def CC_SM83 : CallingConv <[
  CCIfType<[i1], CCPromoteToType<i8>>,
  CCIfType<[i8], CCAssignToReg<[A, L, H, E, D]>>,
  CCIfType<[i16], CCAssignToReg<[HL, DE]>>,
  CCAssignToStack<0, 0>,
]>;

def RetCC_SM83 : CallingConv <[
  CCDelegateTo<CC_SM83>,
]>;

def CSR : CalleeSavedRegs<(add)>;
def CSR_Interrupt : CalleeSavedRegs<(add AF, BC, DE, HL, SP)>;

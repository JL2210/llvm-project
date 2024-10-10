//===- SM83RegisterInfo.td - Describe the SM83 Register File --*- tablegen -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------------===//
//
// This file describes the SM83 Register file, defining the registers themselves,
// aliases between the registers, and the register classes built out of the
// registers.
//
//===------------------------------------------------------------------------===//

class SM83Reg<string name, bits<16> enc = -1, list<Register> subregs = []>
  : RegisterWithSubRegs<name, subregs> {
  let Namespace = "SM83";
  let HWEncoding = enc;
  let SubRegs = subregs;
}

// Subregister indices.
let Namespace = "SM83" in {
  def sub_low : SubRegIndex<8>;
  def sub_high : SubRegIndex<8, 8>;
  def sub_zf : SubRegIndex<1, -1>;
  def sub_cf : SubRegIndex<1, -1>;
}

//===----------------------------------------------------------------------===//
// Register definitions...
//

// 8-bit registers
def B : SM83Reg<"b", 0>;
def C : SM83Reg<"c", 1>;
def D : SM83Reg<"d", 2>;
def E : SM83Reg<"e", 3>;
def H : SM83Reg<"h", 4>;
def L : SM83Reg<"l", 5>;
// dHL is encoded as 6
def A : SM83Reg<"a", 7>;

// N and H omitted due to having no real-world use
def ZF : SM83Reg<"z", 0>;
def CF : SM83Reg<"c", 1>;

let SubRegIndices = [sub_high, sub_low], CoveredBySubRegs = 1 in {
  // 16-bit registers
  def BC : SM83Reg<"bc", 0, [B, C]>;
  def DE : SM83Reg<"de", 1, [D, E]>;
  def HL : SM83Reg<"hl", 2, [H, L]>;
}
let SubRegIndices = [sub_high, sub_zf, sub_cf] in {
  def AF : SM83Reg<"af", 3, [A, ZF, CF]>;
}
// stack pointer
def SP : SM83Reg<"sp", 3>;
// instruction pointer/program counter
// TODO: remove this, it's invisible
def PC : SM83Reg<"pc">;

//===----------------------------------------------------------------------===//
// Register class definitions...
//

// NB: Order matters

let CopyCost = -1 in {
  def CARRY : RegisterClass<"SM83", [i1], 8, (add CF)>;
  def ZERO : RegisterClass<"SM83", [i1], 8, (add ZF)>;
  def FLAGS : RegisterClass<"SM83", [i1], 8, (add CARRY, ZERO)>;
}

def RA : RegisterClass<"SM83", [i8], 8, (add A)>;

// General-purpose 8-bit registers
def GR8 : RegisterClass<"SM83", [i8], 8, (add RA, L, H, E, D, C, B)>;

def RHL : RegisterClass<"SM83", [i16], 8, (add HL)>;

// 16-bit registers (excluding HL) that can be accessed as memory with A
def M16 : RegisterClass<"SM83", [i16], 8, (add DE, BC)>;

// General-purpose 16-bit registers
def GR16 : RegisterClass<"SM83", [i16], 8, (add RHL, M16)>;

// 16-bit registers that can be pushed and popped
def P16 : RegisterClass<"SM83", [i16], 8, (add GR16, AF)>;

// 16 bit registers including sp
def S16 : RegisterClass<"SM83", [i16], 8, (add GR16, SP)>;

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

class SM83Reg<string name,
              bits<16> enc = -1,
              list<Register> subregs = []>
  : RegisterWithSubRegs<name, subregs>
{
  let Namespace = "SM83";
  let HWEncoding = Enc;
  let SubRegs = subregs;
}

// Subregister indices.
let Namespace = "SM83" in {
  def sub_low : SubRegIndex<8>;
  def sub_high : SubRegIndex<8, 8>;
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
// we don't want [hl] to be allocated
def A : SM83Reg<"a", 7>;
def FLAGS : SM83Reg<"f">;

let SubRegIndices = [sub_high, sub_low],
    CoveredBySubRegs = 1 in
{
  // 16-bit registers
  def AF : SM83RegWithSubRegs<"af", 3, [A, FLAGS]>;
  def BC : SM83RegWithSubRegs<"bc", 0, [B, C]>;
  def DE : SM83RegWithSubRegs<"de", 1, [D, E]>;
  def HL : SM83RegWithSubRegs<"hl", 2, [H, L]>;
}
// stack pointer
def SP : SM83Reg<"sp", 3>;
// instruction pointer/program counter
def PC : SM83Reg<"pc">;

// miscellaneous
def IME : SM83Reg<"ime">;

//===----------------------------------------------------------------------===//
// Register class definitions...
//

// General-purpose 8-bit registers which are the low half of a register pair
def GR8lo : RegisterClass<"SM83", [i8], 8, (add C, E, L)>;

// High half of a register pair
def GR8hi : RegisterClass<"SM83", [i8], 8, (add B, D, H)>;

// General-purpose 8-bit registers
def GR8 : RegisterClass<"SM83", [i8], 8, (add A, GR8H, GR8L)>;

// All 8-bit registers
def R8 : RegisterClass<"SM83", [i8], 8, (add GR8, FLAGS)>;

// General-purpose 16-bit registers
def GR16 : RegisterClass<"SM83", [i16], 8, (add BC, DE, HL)>;

// 16-bit registers that can be used with the stack
def SR16 : RegisterClass<"SM83", [i16], 8, (add GR16, AF)>;

// with arithmetic and load operations
def AR16 : RegisterClass<"SM83", [i16], 8, (add GR16, SP)>;

// All 16-bit registers
def R16 : RegisterClass<"SM83", [i16], 8, (add GR16, AF, SP, PC)>;

//===-- SM83InstrInfo.td - SM83 Instruction Descriptions ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes SM83 instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "SM83InstrFormats.td"

class ImmAsmOperand<string name> : AsmOperandClass {
  let Name = name;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "Invalid" # Name;
}

def Condition : AsmOperandClass {
  let Name = "Condition";
  let RenderMethod = "addConditionOperands";
  let DiagnosticType = "Invalid" # Name;
}

def condition : Operand<i8> {
  let ParserMatchClass = Condition;
  let PrintMethod = "printCondition";
}

def rstvec : Operand<i8> {
  let ParserMatchClass = ImmAsmOperand<"RSTVec">;
  let EncoderMethod = "getRSTVecValue";
}

def uimm3 : Operand<i8> {
  let ParserMatchClass = ImmAsmOperand<"UImm3">;
}
def imm8 : Operand<i8> {
  let ParserMatchClass = ImmAsmOperand<"Imm8">;
}
def imm16 : Operand<i16> {
  let ParserMatchClass = ImmAsmOperand<"Imm16">;
}
def rel8 : Operand<i8> {
  let ParserMatchClass = ImmAsmOperand<"Rel8">;
}
def direct16 : Operand<i16> {
  let ParserMatchClass = ImmAsmOperand<"Direct16">;
}
def direct8 : Operand<i8> {
  let ParserMatchClass = ImmAsmOperand<"Direct8">;
}

def NOP : I8<0x00, "nop">;
let hasSideEffects = 1 in
def STOP : SM83Inst16<(outs), (ins), "stop", []> {
    let Inst{7-0} = 0x10;
    let Inst{15-8} = 0x00;
}

def LDrrnn : SM83Inst24<(outs GR16:$dst), (ins imm16:$imm),
             "ld $dst, $imm", [(set GR16:$dst, imm16:$imm)]> {
  bits<2> dst;
  bits<16> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = dst;
  let Inst{3-0} = 0b0001;

  let Inst{23-8} = imm;
}

let Defs = [SP] in
def LDSPnn : SM83Inst24<(outs), (ins imm16:$imm), "ld sp, $imm",
                        [(set SP, imm16:$imm)]> {
  bits<16> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = 0b11;
  let Inst{3-0} = 0b0001;

  let Inst{23-8} = imm;
}

let mayStore = 1 in {
  let Uses = [BC, A] in
  def LDdBCA : I8<0x02, "ld [bc], a", [(store A, BC)]>;
  let Uses = [DE, A] in
  def LDdDEA : I8<0x12, "ld [de], a", [(store A, DE)]>;
}
let mayStore = 1, Uses = [HL, A], Defs = [HL] in {
  def LDdHLIA : I8<0x22, "ld [hl+], a", [(store A, HL), (set HL, (add HL, 1))]>;
  def LDdHLDA : I8<0x32, "ld [hl-], a", [(store A, HL), (set HL, (sub HL, 1))]>;
}

let mayLoad = 1, Defs = [A] in {
  let Uses = [BC] in
  def LDAdBC : I8<0x0a, "ld a, [bc]", [(set A, (load BC))]>;
  let Uses = [DE] in
  def LDAdDE : I8<0x1a, "ld a, [de]", [(set A, (load DE))]>;
}
let mayLoad = 1, Uses = [HL], Defs = [HL, A] in {
  def LDAdHLI : I8<0x2a, "ld a, [hl+]", [(set A, (load HL)), (set HL, (add HL, 1))]>;
  def LDAdHLD : I8<0x3a, "ld a, [hl-]", [(set A, (load HL)), (set HL, (sub HL, 1))]>;
}

let Constraints = "$dst = $src" in
class INC_DECrr<string opstr, bit dec, SDPatternOperator op>
  : SM83Inst8<(outs GR16:$dst), (ins GR16:$src), opstr # " $dst",
              [(set GR16:$dst, (op GR16:$src, 1))]> {
  bits<2> dst;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = dst;
  let Inst{3} = dec;
  let Inst{2-0} = 0b011;
}

def INCrr : INC_DECrr<"inc", 0, add>;
def DECrr : INC_DECrr<"dec", 1, sub>;

let Uses = [SP], Defs = [SP] in {
  def INCSP : I8<0x33, "inc sp", [(set SP, (add SP, (i16 1)))]>;
  def DECSP : I8<0x3b, "dec sp", [(set SP, (sub SP, (i16 1)))]>;
}

let Constraints = "$dst = $src" in
class INC_DECr<string opstr, bit dec, SDPatternOperator op>
  : SM83Inst8<(outs GR8:$dst), (ins GR8:$src), opstr # " $dst",
              [(set GR8:$dst, (op GR8:$src, 1))]> {
  bits<3> dst;

  let Inst{7-6} = 0b00;
  let Inst{5-3} = dst;
  let Inst{2-1} = 0b10;
  let Inst{0} = dec;
}

def INCr : INC_DECr<"inc", 0, add>;
def DECr : INC_DECr<"dec", 1, sub>;

let Uses = [HL], mayLoad = 1, mayStore = 1 in {
  def INCdHL : I8<0x34, "inc [hl]", [(store (add (i8 (load HL)), 1), HL)]>;
  def DECdHL : I8<0x35, "dec [hl]", [(store (sub (i8 (load HL)), 1), HL)]>;
}

def LDri : SM83Inst16<(outs GR8:$dst), (ins imm8:$imm), "ld $dst, $imm",
                     [(set GR8:$dst, imm8:$imm)]> {
  bits<3> dst;
  bits<8> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-3} = dst;
  let Inst{2-0} = 0b110;

  let Inst{15-8} = imm;
}

let mayStore = 1, Uses = [HL] in
def LDdHLi : SM83Inst16<(outs), (ins imm8:$imm), "ld [hl], $imm",
                       [(store imm8:$imm, HL)]> {
  bits<8> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-3} = 0b110;
  let Inst{2-0} = 0b110;

  let Inst{15-8} = imm;
}

let Uses = [A], Defs = [A, FLAGS], AddedComplexity = 1 in {
  def RLCA : I8<0x07, "rlca", [(set A, (rotl A, (i8 1)))]>;
  def RRCA : I8<0x0f, "rrca", [(set A, (rotr A, (i8 1)))]>;
}
let Uses = [A], Defs = [A, FLAGS] in {
  def RLA : I8<0x17, "rla">;
  def RRA : I8<0x1f, "rra">;
  def DAA : I8<0x27, "daa">;
  def CPL : I8<0x2f, "cpl", [(set A, (not A))]>;
}
let Defs = [FLAGS] in {
  def SCF : I8<0x37, "scf">;
  def CCF : I8<0x3f, "ccf">;
}

let mayStore = 1, Uses = [SP] in
def LDd16SP : SM83Inst24<(outs), (ins direct16:$addr), "ld [$addr], sp",
                         [(store SP, direct16:$addr)]> {
  bits<16> addr;

  let Inst{7-0} = 0x08;
  let Inst{23-8} = addr;
}

def JRr8 : SM83Inst16<(outs), (ins rel8:$off), "jr $off", []> {
  bits<8> off;

  let Inst{7-0} = 0x18;
  let Inst{15-8} = off;

  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
}

def JRccr8 : SM83Inst16<(outs), (ins condition:$cc, rel8:$off),
                        "jr $cc, $off", []> {
  bits<2> cc;

  let Inst{7-5} = 0b001;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b000;

  let isBranch = 1;
  let isTerminator = 1;
}

let Uses = [HL], Defs = [HL] in
def ADDHLrr : SM83Inst8<(outs), (ins GR16:$src), "add hl, $src",
                        [(set HL, (add HL, GR16:$src))]> {
  bits<2> src;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = src;
  let Inst{3-0} = 0b1001;
}
let Uses = [HL, SP], Defs = [HL] in
def ADDHLSP : I8<0x39, "add hl, sp", [(set HL, (add HL, SP))]>;

def LDrr : SM83Inst8<(outs GR8:$dst), (ins GR8:$src), "ld $dst, $src",
                     [(set GR8:$dst, GR8:$src)]> {
  bits<3> dst;
  bits<3> src;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = dst;
  let Inst{2-0} = src;
}

let Uses = [HL], mayStore = 1 in
def LDdHLr : SM83Inst8<(outs), (ins GR8:$src), "ld [hl], $src",
                       [(store GR8:$src, HL)]> {
  bits<3> src;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = 0b110;
  let Inst{2-0} = src;
}

let Uses = [HL], mayLoad = 1 in
def LDrdHL : SM83Inst8<(outs GR8:$dst), (ins), "ld $dst, [hl]",
                       [(set GR8:$dst, (load HL))]> {
  bits<3> dst;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = dst;
  let Inst{2-0} = 0b110;
}

// not ld [hl], [hl]
let hasSideEffects = 1 in
def HALT : I8<0x76, "halt">;

multiclass mALU<string opstr, bits<3> opc, list<SM83Reg> uses,
                SDPatternOperator op = null_frag> {
  let Uses = uses in {
    def r : SM83Inst8<(outs), (ins GR8:$src), opstr # " a, $src",
                      [(set A, (op A, GR8:$src))]> {
      bits<3> src;

      let Inst{7-6} = 0b10;
      let Inst{5-3} = opc;
      let Inst{2-0} = src;
    }
    def i : SM83Inst16<(outs), (ins imm8:$imm), opstr # " a, $imm",
                       [(set A, (op A, imm8:$imm))]> {
      bits<8> imm;

      let Inst{7-6}  = 0b11;
      let Inst{5-3}  = opc;
      let Inst{2-0}  = 0b110;
      let Inst{15-8} = imm;
    }
  }

  let Uses = !listconcat(uses, [HL]), mayLoad = 1 in
  def dHL : SM83Inst8<(outs), (ins), opstr # " a, [hl]",
                      [(set A, (op A, (load HL)))]> {
    let Inst{7-6} = 0b10;
    let Inst{5-3} = opc;
    let Inst{2-0} = 0b110;
  }
}

let Defs = [A, FLAGS] in {
  defm ADD : mALU<"add", 0b000, [A], add>;
  defm SUB : mALU<"sub", 0b010, [A], sub>;
  defm AND : mALU<"and", 0b100, [A], and>;
  defm XOR : mALU<"xor", 0b101, [A], xor>;
  defm OR : mALU<"or", 0b110, [A], or>;

  defm ADC : mALU<"adc", 0b001, [A, FLAGS]>;
  defm SBC : mALU<"sbc", 0b011, [A, FLAGS]>;
}
let Defs = [FLAGS] in
defm CP : mALU<"cp", 0b111, [A]>;

let isReturn = 1 in
def RETcc : SM83Inst8<(outs), (ins condition:$cc), "ret $cc", []> {
  bits<2> cc;

  let Inst{7-5} = 0b110;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b000;
}

let Uses = [A], mayStore = 1, AddedComplexity = 1 in
def LDHd8A : SM83Inst16<(outs), (ins direct8:$addr), "ldh [$addr], a",
                        [(store A, direct8:$addr)]> {
  bits<8> addr;

  let Inst{7-0} = 0b11100000;
  let Inst{15-8} = addr;
}

let Defs = [A], mayLoad = 1, AddedComplexity = 1 in
def LDHAd8 : SM83Inst16<(outs), (ins direct8:$addr), "ldh a, [$addr]",
                        [(set A, (load direct8:$addr))]> {
  bits<8> addr;

  let Inst{7-0} = 0b11110000;
  let Inst{15-8} = addr;
}

let mayLoad = 1 in {
  def POPrr : SM83Inst8<(outs GR16:$dst), (ins), "pop $dst", []> {
    bits<2> dst;

    let Inst{7-6} = 0b11;
    let Inst{5-4} = dst;
    let Inst{3-0} = 0b0001;
  }

  let Defs = [AF] in
  def POPAF : I8<0xf1, "pop af">;
}

let mayStore = 1 in {
  def PUSHrr : SM83Inst8<(outs), (ins GR16:$src), "push $src", []> {
    bits<2> src;

    let Inst{7-6} = 0b11;
    let Inst{5-4} = src;
    let Inst{3-0} = 0b0101;
  }

  let Uses = [AF] in
  def PUSHAF : I8<0xf5, "push af">;
}

def JPccd16 : SM83Inst24<(outs), (ins condition:$cc, direct16:$addr),
                         "jp $cc, $addr", []> {
  bits<2> cc;
  bits<16> addr;

  let Inst{7-5} = 0b110;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b010;
}

let isCall = 1 in
def RST : SM83Inst8<(outs), (ins rstvec:$vec), "rst $vec", []> {
  bits<3> vec;

  let Inst{7-6} = 0b11;
  let Inst{5-3} = vec;
  let Inst{2-0} = 0b111;
}

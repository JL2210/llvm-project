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
def simm8 : Operand<i8> {
  let ParserMatchClass = ImmAsmOperand<"SImm8">;
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
def direct8 : Operand<i8> {
  let ParserMatchClass = ImmAsmOperand<"Direct8">;
  let EncoderMethod = "getDirect8Value";
}
def direct16 : Operand<i16> {
  let ParserMatchClass = ImmAsmOperand<"Direct16">;
}

def NOP : I8<0x00, "nop">;
let hasSideEffects = 1 in
def STOP : SM83Inst16<(outs), (ins), "stop", []> {
  let Inst{7-0} = 0x10;
  // stop needs to have an extra byte; otherwise,
  // the first byte of the next instruction is skipped
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
  def LDdHLIA : I8<0x22, "ld [hli], a", [(store A, HL), (set HL, (add HL, 1))]>;
  def LDdHLDA : I8<0x32, "ld [hld], a", [(store A, HL), (set HL, (sub HL, 1))]>;
}

let mayLoad = 1, Defs = [A] in {
  let Uses = [BC] in
  def LDAdBC : I8<0x0a, "ld a, [bc]", [(set A, (load BC))]>;
  let Uses = [DE] in
  def LDAdDE : I8<0x1a, "ld a, [de]", [(set A, (load DE))]>;
}
let mayLoad = 1, Uses = [HL], Defs = [HL, A] in {
  def LDAdHLI : I8<0x2a, "ld a, [hli]", [(set A, (load HL)), (set HL, (add HL, 1))]>;
  def LDAdHLD : I8<0x3a, "ld a, [hld]", [(set A, (load HL)), (set HL, (sub HL, 1))]>;
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

let Constraints = "$dst = $src", Defs = [FLAGS] in
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

let Uses = [HL], Defs = [FLAGS], mayLoad = 1, mayStore = 1 in {
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

let isTerminator = 1, isBranch = 1, isBarrier = 1 in
def JRr8 : SM83Inst16<(outs), (ins rel8:$off), "jr $off", []> {
  bits<8> off;

  let Inst{7-0} = 0x18;
  let Inst{15-8} = off;
}
let isTerminator = 1, isBranch = 1 in
def JRccr8 : SM83Inst16<(outs), (ins condition:$cc, rel8:$off),
                        "jr $cc, $off", []> {
  bits<2> cc;

  let Inst{7-5} = 0b001;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b000;
}

let Uses = [HL], Defs = [HL, FLAGS] in
def ADDHLrr : SM83Inst8<(outs), (ins GR16:$src), "add hl, $src",
                        [(set HL, (add HL, GR16:$src))]> {
  bits<2> src;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = src;
  let Inst{3-0} = 0b1001;
}
let Uses = [HL, SP], Defs = [HL, FLAGS] in
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

let isReturn = 1, isTerminator = 1 in
def RETcc : SM83Inst8<(outs), (ins condition:$cc), "ret $cc", []> {
  bits<2> cc;

  let Inst{7-5} = 0b110;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b000;
}
let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RET : I8<0xc9, "ret">;
  let hasSideEffects = 1 in
  def RETI : I8<0xd9, "reti">;
}

let AddedComplexity = 1 in
class ldhd8<bit reverse, string asmstr, list<dag> pattern>
  : SM83Inst16<(outs), (ins direct8:$addr), asmstr, pattern> {
  bits<8> addr;

  let Inst{7-5} = 0b111;
  let Inst{4} = reverse;
  let Inst{3-0} = 0b0000;

  let Inst{15-8} = addr;
}

let Uses = [A], mayStore = 1 in
def LDHd8A : ldhd8<0, "ldh [$addr], a", [(store A, direct8:$addr)]>;
let Defs = [A], mayLoad = 1 in
def LDHAd8 : ldhd8<1, "ldh a, [$addr]", [(set A, (load direct8:$addr))]>;

let Uses = [C, A], mayStore = 1 in
def LDHdCA : I8<0xe2, "ldh [c], a", [(store A, (i16 (or 0xff00, C)))]>;
let Uses = [C], Defs = [A], mayLoad = 1 in
def LDHAdC : I8<0xf2, "ldh a, [c]", [(set A, (load (i16 (or 0xff00, C))))]>;

let Defs = [SP] in {
  def POPrr : SM83Inst8<(outs GR16:$dst), (ins), "pop $dst", []> {
    bits<2> dst;

    let Inst{7-6} = 0b11;
    let Inst{5-4} = dst;
    let Inst{3-0} = 0b0001;
  }

  let Defs = [AF] in
  def POPAF : I8<0xf1, "pop af">;

  def PUSHrr : SM83Inst8<(outs), (ins GR16:$src), "push $src", []> {
    bits<2> src;

    let Inst{7-6} = 0b11;
    let Inst{5-4} = src;
    let Inst{3-0} = 0b0101;
  }

  let Uses = [AF] in
  def PUSHAF : I8<0xf5, "push af">;
}

let isBranch = 1, isTerminator = 1 in
def JPccd16 : SM83Inst24<(outs), (ins condition:$cc, direct16:$addr),
                         "jp $cc, $addr", []> {
  bits<2> cc;
  bits<16> addr;

  let Inst{7-5} = 0b110;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b010;
}
let isBranch = 1, isTerminator = 1, isBarrier = 1 in
def JPd16 : SM83Inst24<(outs), (ins direct16:$addr), "jp $addr", []> {
  bits<16> addr;

  let Inst{7-0} = 0xc3;
  let Inst{23-8} = addr;
}

let hasSideEffects = 1 in {
  def DI : I8<0xf3, "di">;
  def EI : I8<0xfb, "ei">;
}

let isTerminator = 1, isCall = 1 in {
  def CALLccd16 : SM83Inst24<(outs), (ins condition:$cc, direct16:$addr),
                             "call $addr", []> {
    bits<2> cc;
    bits<16> addr;

    let Inst{7-5} = 0b110;
    let Inst{4-3} = cc;
    let Inst{2-0} = 0b100;

    let Inst{23-8} = addr;
  }

  def CALLd16 : SM83Inst24<(outs), (ins direct16:$addr), "call $addr", []> {
    bits<16> addr;

    let Inst{7-0} = 0xcd;
    let Inst{23-8} = addr;
  }
}

let Uses = [SP], Defs = [SP, FLAGS] in
def ADDSPs8 : SM83Inst16<(outs), (ins simm8:$s8), "add sp, $s8",
                         [(set SP, (add SP, simm8:$s8))]> {
  bits<8> s8;

  let Inst{7-0} = 0xe8;
  let Inst{15-8} = s8;
}

let isIndirectBranch = 1, isBranch = 1, isTerminator = 1, isBarrier = 1 in
def JPHL : I8<0xe9, "jp hl">;

let Uses = [SP], Defs = [HL, FLAGS] in
def LDHLSPs8 : SM83Inst16<(outs), (ins simm8:$s8), "ld hl, sp+$s8",
                          [(set HL, (add SP, simm8:$s8))]> {
  bits<8> s8;

  let Inst{7-0} = 0xf8;
  let Inst{15-8} = s8;
}

let Defs = [SP], Uses = [HL] in
def LDSPHL : I8<0xf9, "ld sp, hl", [(set SP, HL)]>;

let isCall = 1 in
def RST : SM83Inst8<(outs), (ins rstvec:$vec), "rst $vec", []> {
  bits<3> vec;

  let Inst{7-6} = 0b11;
  let Inst{5-3} = vec;
  let Inst{2-0} = 0b111;
}

multiclass mBIT<string opstr, bits<3> opc, SDPatternOperator op = null_frag, int n = 1> {
  let Constraints = "$src = $dst", Defs = [FLAGS] in
  def r : CB<(outs GR8:$dst), (ins GR8:$src), opstr # " $dst",
             [(set GR8:$dst, (op GR8:$src, (i8 n)))]> {
    bits<3> dst;

    let Inst{7-6} = 0b00;
    let Inst{5-3} = opc;
    let Inst{2-0} = dst;
  }

  let Uses = [HL], Defs = [FLAGS], mayLoad = 1, mayStore = 1 in
  def dHL : CB<(outs), (ins), opstr # " [hl]",        
             [(store (op (i8 (load HL)), (i8 n)), HL)]> {
    let Inst{7-6} = 0b00;
    let Inst{5-3} = opc;
    let Inst{2-0} = 0b110;
  }
}

defm RLC : mBIT<"rlc", 0b000, rotl>;
defm RRC : mBIT<"rrc", 0b001, rotr>;
defm RL : mBIT<"rl", 0b010>;
defm RR : mBIT<"rr", 0b011>;
defm SLA : mBIT<"sla", 0b100, shl>;
defm SRA : mBIT<"sra", 0b101, sra>;
defm SWAP : mBIT<"swap", 0b110, rotr, 4>;
defm SRL : mBIT<"srl", 0b111, srl>;

def setbitn : PatFrag<(ops node:$in, node:$idx),
                      (or node:$in, (shl 1, node:$idx))>;
def resbitn : PatFrag<(ops node:$in, node:$idx),
                      (and node:$in, (not (shl 1, node:$idx)))>;

// bit reset & set
multiclass mBRS<string opstr, bits<2> opc, SDPatternOperator op> {
  let Constraints = "$src = $dst" in
  def r : CB<(outs GR8:$dst), (ins uimm3:$idx, GR8:$src), opstr # " $idx, $dst",
             [(set GR8:$dst, (op GR8:$src, uimm3:$idx))]> {
    bits<3> idx;
    bits<3> dst;

    let Inst{7-6} = opc;
    let Inst{5-3} = idx;
    let Inst{2-0} = dst;
  }

  let Uses = [HL], mayLoad = 1, mayStore = 1 in
  def dHL : CB<(outs), (ins uimm3:$idx), opstr # " $idx, [hl]",
               [(store (op (i8 (load HL)), uimm3:$idx), HL)]> {
    bits<3> idx;

    let Inst{7-6} = opc;
    let Inst{5-3} = idx;
    let Inst{2-0} = 0b110;
  }
}

defm RES : mBRS<"res", 0b10, resbitn>;
defm SET : mBRS<"set", 0b11, setbitn>;

let Defs = [FLAGS] in
def BITr : CB<(outs), (ins uimm3:$idx, GR8:$src), "bit $idx, $src", []> {
  bits<3> idx;
  bits<3> src;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = idx;
  let Inst{2-0} = src;
}

let Uses = [HL], Defs = [FLAGS], mayLoad = 1 in
def BITdHL : CB<(outs), (ins uimm3:$idx), "bit $idx, [hl]", []> {
  bits<3> idx;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = idx;
  let Inst{2-0} = 0b110;
}

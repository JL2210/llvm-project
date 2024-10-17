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

def condition : Operand<i8> {
  let PrintMethod = "printCondition";
  let OperandNamespace = "SM83";
  let OperandType = "OPERAND_CONDITION";
}

def SM83_COND_NZ : PatLeaf<(i8 0)>;
def SM83_COND_Z : PatLeaf<(i8 1)>;
def SM83_COND_NC : PatLeaf<(i8 2)>;
def SM83_COND_C : PatLeaf<(i8 3)>;

def rstvec : Operand<i16>, ImmLeaf<i16, [{
  return isShiftedUInt<3, 3>(Imm);
}]> {
}

def uimm3 : Operand<i8>, ImmLeaf<i8, [{
  return isUInt<3>(Imm);
}]> {
}
def simm8 : Operand<i8>, ImmLeaf<i8, [{
  return isInt<8>(Imm);
}]> {
}
def imm8 : Operand<i8>, ImmLeaf<i8, [{
  return isUInt<8>(Imm) || isInt<8>(Imm);
}]> {
}
def imm16 : Operand<i16>, ImmLeaf<i16, [{
  return isUInt<16>(Imm) || isInt<16>(Imm);
}]> {
}
def direct8 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0xff00 && Imm <= 0xffff;
}]> {
}
def direct16 : Operand<i16>, ImmLeaf<i16, [{
  return isUInt<16>(Imm);
}]> {
}

def frameidx : Operand<iPTR> {
  let MIOperandInfo = (ops ptr_rc/*base*/, i16imm/*disp*/);
  let PrintMethod = "printMemOp";
  let OperandType = "OPERAND_MEMORY";
}

def off : Operand<iPTR>;

def relbrtarget : Operand<OtherVT>;
def brtarget : Operand<OtherVT>;

// LLVM likes to select instructions in the order it sees them;
// but apparently it just doesn't like to load or
// store via immediate addresses so these are useless
let Uses = [A], mayStore = 1 in
def LDd16A : SM83Inst24<(outs), (ins direct16:$addr), "ld [$addr], a",
                        [(store A, direct16:$addr)]> {
  bits<16> addr;

  let Inst{7-0} = 0xea;
  let Inst{23-8} = addr;
}

let Defs = [A], mayLoad = 1 in
def LDAd16 : SM83Inst24<(outs), (ins direct16:$addr), "ld a, [$addr]",
                        [(set A, (i8 (load direct16:$addr)))]> {
  bits<16> addr;

  let Inst{7-0} = 0xfa;
  let Inst{23-8} = addr;
}

def NOP : I8<0x00, "nop">;

let hasSideEffects = 1 in
def STOP : I8<0x10, "stop">;

let isMoveImm = 1, isReMaterializable = 1, isAsCheapAsAMove = 1 in
def LDrrii : SM83Inst24<(outs S16:$dst), (ins imm16:$imm),
             "ld $dst, $imm", [(set S16:$dst, imm16:$imm)]> {
  bits<2> dst;
  bits<16> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = dst;
  let Inst{3-0} = 0b0001;

  let Inst{23-8} = imm;
}

let Uses = [A], mayStore = 1 in {
  def LDdHLIA : I8<0x22, "ld [$hl+], a", [(set RHL:$hl, (post_store A, RHL:$dstp, 1))],
                   (outs RHL:$hl), (ins RHL:$dstp)>;
  def LDdHLDA : I8<0x32, "ld [$hl-], a", [(set RHL:$hl, (post_store A, RHL:$dstp, -1))],
                   (outs RHL:$hl), (ins RHL:$dstp)>;
}

let Uses = [HL], Defs = [HL, A], mayLoad = 1 in {
  // patterns can't have two outputs, so these will have to be selected manually
  // therefore, no reason to bother putting them into a form that suits automatic selection
  def LDAdHLI : I8<0x2a, "ld a, [hli]", []>;
  def LDAdHLD : I8<0x3a, "ld a, [hld]", []>;
}

let Constraints = "$dst = $src" in
class INC_DECrr<string opstr, bit dec, SDPatternOperator op>
  : SM83Inst8<(outs S16:$dst), (ins S16:$src), opstr # " $dst",
              [(set S16:$dst, (op S16:$src, 1))]> {
  bits<2> dst;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = dst;
  let Inst{3} = dec;
  let Inst{2-0} = 0b011;
}

def INCrr : INC_DECrr<"inc", 0, add>;
def DECrr : INC_DECrr<"dec", 1, sub>;

let Constraints = "$dst = $src", Defs = [ZF] in
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

let Defs = [ZF], mayLoad = 1, mayStore = 1 in {
  def INCdHL : I8<0x34, "inc [$hl]", [(store (add (i8 (load RHL:$hl)), 1), RHL:$hl)],
                  (outs), (ins RHL:$hl)>;
  def DECdHL : I8<0x35, "dec [$hl]", [(store (sub (i8 (load RHL:$hl)), 1), RHL:$hl)],
                  (outs), (ins RHL:$hl)>;
}

let isMoveImm = 1, isReMaterializable = 1, isAsCheapAsAMove = 1 in
def LDri : SM83Inst16<(outs GR8:$dst), (ins imm8:$imm), "ld $dst, $imm",
                      [(set GR8:$dst, imm8:$imm)]> {
  bits<3> dst;
  bits<8> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-3} = dst;
  let Inst{2-0} = 0b110;

  let Inst{15-8} = imm;
}

let mayStore = 1, isMoveImm = 1 in
def LDdHLi : SM83Inst16<(outs), (ins RHL:$hl, imm8:$imm), "ld [$hl], $imm",
                        [(store imm8:$imm, RHL:$hl)]> {
  bits<8> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-3} = 0b110;
  let Inst{2-0} = 0b110;

  let Inst{15-8} = imm;
}

// AddedComplexity > 0 makes the instruction more likely to be selected
let Uses = [A], Defs = [A, ZF, CF], AddedComplexity = 1 in {
  def RLCA : I8<0x07, "rlca", [(set A, (rotl A, (i8 1)))]>;
  def RRCA : I8<0x0f, "rrca", [(set A, (rotr A, (i8 1)))]>;
}
let Uses = [A, CF], Defs = [A, ZF, CF] in {
  // NOTE: use these for 16-bit shift/rotate

  def RLA : I8<0x17, "rla",
  [(set A,
     (or
       (shl A, (i8 1)),
       (i8 (zext CF))
     )
   )], (outs), (ins)>;

  def RRA : I8<0x1f, "rra",
  [(set A,
     (or
       (srl A, (i8 1)),
       (shl
         (i8 (zext CF)),
         (i8 7)
       )
     )
   )], (outs), (ins)>;
}

// the one instruction that uses n and h
let Uses = [AF], Defs = [AF] in
def DAA : I8<0x27, "daa">;

// doesn't set any "useful" flags
let Uses = [A], Defs = [A] in
def CPL : I8<0x2f, "cpl", [(set A, (not A))],
             (outs), (ins)>;

let Defs = [CF] in {
  def SCF : I8<0x37, "scf", [(set CF, (i1 1))]>;
  let Uses = [CF] in
  def CCF : I8<0x3f, "ccf", [(set CF, (not CF))]>;
}

let mayStore = 1, Uses = [SP] in
def LDd16SP : SM83Inst24<(outs), (ins direct16:$addr), "ld [$addr], sp",
                         [(store SP, direct16:$addr)]> {
  bits<16> addr;

  let Inst{7-0} = 0x08;
  let Inst{23-8} = addr;
}

let isTerminator = 1, isBranch = 1, isBarrier = 1 in
def JRr8 : SM83Inst16<(outs), (ins relbrtarget:$off), "jr $off", [(br bb:$off)]> {
  bits<8> off;

  let Inst{7-0} = 0x18;
  let Inst{15-8} = off;
}
let isTerminator = 1, isBranch = 1, Uses = [ZF, CF] in
def JRccr8 : SM83Inst16<(outs), (ins condition:$cc, relbrtarget:$off),
                        "jr $cc, $off", []> {
  bits<2> cc;
  bits<8> off;

  let Inst{7-5} = 0b001;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b000;
  let Inst{15-8} = off;
}

let Defs = [CF], Constraints = "$dst = $hl" in
def ADDHLrr : SM83Inst8<(outs RHL:$dst), (ins RHL:$hl, S16:$src), "add $hl, $src",
                        [(set RHL:$dst, (add RHL:$hl, S16:$src))]> {
  bits<2> src;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = src;
  let Inst{3-0} = 0b1001;
}

let isMoveReg = 1 in
def LDrr : SM83Inst8<(outs GR8:$dst), (ins GR8:$src), "ld $dst, $src",
                     [(set GR8:$dst, GR8:$src)]> {
  bits<3> dst;
  bits<3> src;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = dst;
  let Inst{2-0} = src;
}

let mayStore = 1 in
def LDdHLr : SM83Inst8<(outs), (ins GR8:$src, RHL:$hl), "ld [$hl], $src",
                       [(store GR8:$src, RHL:$hl)]> {
  bits<3> src;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = 0b110;
  let Inst{2-0} = src;
}

let mayLoad = 1 in
def LDrdHL : SM83Inst8<(outs GR8:$dst), (ins RHL:$hl), "ld $dst, [$hl]",
                       [(set GR8:$dst, (load RHL:$hl))]> {
  bits<3> dst;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = dst;
  let Inst{2-0} = 0b110;
}

let Uses = [A], mayStore = 1 in
def LDdrrA : SM83Inst8<(outs), (ins M16:$dstp), "ld [$dstp], a",
                       [(store A, M16:$dstp)]> {
  bits<2> dstp;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = dstp;
  let Inst{3-0} = 0b0010;
}

let Defs = [A], mayLoad = 1 in
def LDAdrr : SM83Inst8<(outs), (ins M16:$srcp), "ld a, [$srcp]",
                       [(set A, (load M16:$srcp))]> {
  bits<2> dstp;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = dstp;
  let Inst{3-0} = 0b1010;
}

// not ld [hl], [hl]
let hasSideEffects = 1 in
def HALT : I8<0x76, "halt">;

let Uses = [A], Defs = [A, ZF, CF] in {
  multiclass mALU<string opstr, bits<3> opc, SDPatternOperator op = null_frag> {
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

    let mayLoad = 1 in
    def dHL : SM83Inst8<(outs), (ins RHL:$hl), opstr # " a, [$hl]",
                        [(set A, (op A, (load RHL:$hl)))]> {
      let Inst{7-6} = 0b10;
      let Inst{5-3} = opc;
      let Inst{2-0} = 0b110;
    }
  }
}

defm ADD : mALU<"add", 0b000, add>;
defm SUB : mALU<"sub", 0b010, sub>;
defm AND : mALU<"and", 0b100, and>;
defm XOR : mALU<"xor", 0b101, xor>;
defm OR : mALU<"or", 0b110, or>;

let Uses = [A, CF] in {
  // Why can't these be selected? Sigh.
  defm ADC : mALU<"adc", 0b001>;
  defm SBC : mALU<"sbc", 0b011>;
}

let isReturn = 1, isTerminator = 1, Uses = [ZF, CF] in
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

let Uses = [A], mayStore = 1, AddedComplexity = 1 in
def LDHd8A : SM83Inst16<(outs), (ins direct8:$addr), "ldh [$addr], a",
                        [(store A, direct8:$addr)]> {
  bits<16> addr;
  let Inst{7-0} = 0b11100000;
  let Inst{15-8} = addr{7-0};
}
let Defs = [A], mayLoad = 1, AddedComplexity = 1 in
def LDHAd8 : SM83Inst16<(outs), (ins direct8:$addr), "ldh a, [$addr]",
                        [(set A, (load direct8:$addr))]> {
  bits<16> addr;
  let Inst{7-0} = 0b11110000;
  let Inst{15-8} = addr{7-0};
}

let Uses = [A], mayStore = 1 in
def LDHdCA : I8<0xe2, "ldh [$c], a", [(store A, (addrspacecast RC:$c))], (outs), (ins RC:$c)>;
let Defs = [A], mayLoad = 1 in
def LDHAdC : I8<0xf2, "ldh a, [$c]", [(set A, (load (addrspacecast RC:$c)))], (outs), (ins RC:$c)>;

let Uses = [SP], Defs = [SP] in {
  let mayLoad = 1, mayStore = 1 in
  def POPrr : SM83Inst8<(outs P16:$dst), (ins), "pop $dst", []> {
    bits<2> dst;

    let Inst{7-6} = 0b11;
    let Inst{5-4} = dst;
    let Inst{3-0} = 0b0001;
  }

  let mayStore = 1 in
  def PUSHrr : SM83Inst8<(outs), (ins P16:$src), "push $src", []> {
    bits<2> src;

    let Inst{7-6} = 0b11;
    let Inst{5-4} = src;
    let Inst{3-0} = 0b0101;
  }
}

let Uses = [ZF, CF], isBranch = 1, isTerminator = 1 in
def JPcc : SM83Inst24<(outs), (ins condition:$cc, brtarget:$addr), "jp $cc, $addr", []> {
  bits<2> cc;
  bits<16> addr;

  let Inst{7-5} = 0b110;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b010;
}

let isBranch = 1, isTerminator = 1, isBarrier = 1 in
def JPd16 : SM83Inst24<(outs), (ins brtarget:$addr), "jp $addr", [(br bb:$addr)]> {
  bits<16> addr;

  let Inst{7-0} = 0xc3;
  let Inst{23-8} = addr;
}

let hasSideEffects = 1 in {
  def DI : I8<0xf3, "di">;
  def EI : I8<0xfb, "ei">;
}

let isCall = 1 in {
  def CALLccd16 : SM83Inst24<(outs), (ins condition:$cc, brtarget:$addr),
                             "call $cc, $addr", []> {
    bits<2> cc;
    bits<16> addr;

    let Inst{7-5} = 0b110;
    let Inst{4-3} = cc;
    let Inst{2-0} = 0b100;

    let Inst{23-8} = addr;
  }

  def CALLd16 : SM83Inst24<(outs), (ins brtarget:$addr), "call $addr", []> {
    bits<16> addr;

    let Inst{7-0} = 0xcd;
    let Inst{23-8} = addr;
  }
}

let Uses = [SP], Defs = [SP, ZF, CF] in
def ADDSPs8 : SM83Inst16<(outs), (ins simm8:$s8), "add sp, $s8", []> {
  bits<8> s8;

  let Inst{7-0} = 0xe8;
  let Inst{15-8} = s8;
}

let isIndirectBranch = 1, isBranch = 1, isTerminator = 1, isBarrier = 1 in
def JPHL : I8<0xe9, "jp hl", [(brind RHL:$hl)], (outs), (ins RHL:$hl)>;

let Uses = [SP], Defs = [ZF, CF] in
def LDHLSPs8 : SM83Inst16<(outs RHL:$hl), (ins simm8:$s8), "ld hl, sp+$s8", []> {
  bits<8> s8;

  let Inst{7-0} = 0xf8;
  let Inst{15-8} = s8;
}

let Defs = [SP] in
def LDSPHL : I8<0xf9, "ld sp, hl", [], (outs), (ins RHL:$hl)>;

let isCall = 1 in
def RST : SM83Inst8<(outs), (ins rstvec:$vec), "rst $vec", []> {
  bits<3> vec;

  let Inst{7-6} = 0b11;
  let Inst{5-3} = vec;
  let Inst{2-0} = 0b111;
}

// XXX: this assumes that rst 38 is always an error handler
let isCall = 1, isTrap = 1 in
def RST38cc : SM83Inst8<(outs), (ins condition:$cc), "jr $cc, @-1", []>;

let Defs = [ZF, CF] in
multiclass mBIT<string opstr, bits<3> opc, SDPatternOperator op = null_frag, int n = 1> {
  let Constraints = "$src = $dst" in
  def r : CB<(outs GR8:$dst), (ins GR8:$src), opstr # " $dst",
             [(set GR8:$dst, (op GR8:$src, (i8 n)))]> {
    bits<3> dst;

    let Inst{7-6} = 0b00;
    let Inst{5-3} = opc;
    let Inst{2-0} = dst;
  }

  let mayLoad = 1, mayStore = 1 in
  def dHL : CB<(outs), (ins RHL:$hl), opstr # " [$hl]",
             [(store (op (i8 (load RHL:$hl)), (i8 n)), RHL:$hl)]> {
    let Inst{7-6} = 0b00;
    let Inst{5-3} = opc;
    let Inst{2-0} = 0b110;
  }
}

defm RLC : mBIT<"rlc", 0b000, rotl>;
defm RRC : mBIT<"rrc", 0b001, rotr>;
let Uses = [CF] in {
  defm RL : mBIT<"rl", 0b010>;
  defm RR : mBIT<"rr", 0b011>;
}
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
  def dHL : CB<(outs), (ins uimm3:$idx, RHL:$hl), opstr # " $idx, [$hl]",
               [(store (op (i8 (load RHL:$hl)), uimm3:$idx), RHL:$hl)]> {
    bits<3> idx;

    let Inst{7-6} = opc;
    let Inst{5-3} = idx;
    let Inst{2-0} = 0b110;
  }
}

defm RES : mBRS<"res", 0b10, resbitn>;
defm SET : mBRS<"set", 0b11, setbitn>;

def BITr : CB<(outs ZERO:$zf), (ins uimm3:$idx, GR8:$src), "bit $idx, $src",
              [(set ZERO:$zf, (trunc (srl GR8:$src, uimm3:$idx)))]> {
  bits<3> idx;
  bits<3> src;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = idx;
  let Inst{2-0} = src;
}

let mayLoad = 1 in
def BITdHL : CB<(outs ZERO:$zf), (ins uimm3:$idx, RHL:$hl), "bit $idx, [$hl]",
                [(set ZERO:$zf, (trunc (srl (i8 (load RHL:$hl)), uimm3:$idx)))]> {
  bits<3> idx;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = idx;
  let Inst{2-0} = 0b110;
}

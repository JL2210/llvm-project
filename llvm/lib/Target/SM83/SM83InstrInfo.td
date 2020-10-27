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

def LDrrnn : SM83Inst24<(outs GR16:$dst), (ins i16imm:$imm),
             "ld $dst, $imm", [(set GR16:$dst, i16imm:$imm)]> {
    bits<2> dst;
    bits<16> imm;

    let Inst{7-6} = 0b00;
    let Inst{5-4} = dst;
    let Inst{3-0} = 0b0001;

    let Inst{23-8} = imm;
}

let Defs = [SP] in
def LDSPnn : SM83Inst24<(outs), (ins i16imm:$imm), "ld sp, $imm",
                        [(set SP, i16imm:$imm)]> {
    bits<16> imm;

    let Inst{7-0} = 0x31;
    let Inst{23-8} = imm;
}

let mayStore = 1 in {
    let Uses = [BC, A] in
    def LDdBCA : I8<0x02, "ld [bc], a", [(store BC, A)]>;
    let Uses = [DE, A] in
    def LDdDEA : I8<0x12, "ld [de], a", [(store DE, A)]>;
    let Uses = [HL, A], Defs = [HL] in {
        def LDdHLIA : I8<0x22, "ld [hl+], a", [(store HL, A), (set HL, (add HL, 1))]>;
        def LDdHLDA : I8<0x32, "ld [hl-], a", [(store HL, A), (set HL, (sub HL, 1))]>;
    }
}

let mayLoad = 1, Defs = [A] in {
    let Uses = [BC] in
    def LDAdBC : I8<0x0a, "ld a, [bc]", [(set A, (load BC))]>;
    let Uses = [DE] in
    def LDAdDE : I8<0x1a, "ld a, [de]", [(set A, (load DE))]>;
    let Uses = [HL], Defs = [HL, A] in {
        def LDAdHLI : I8<0x2a, "ld a, [hl+]", [(set A, (load HL)), (set HL, (add HL, 1))]>;
        def LDAdHLD : I8<0x3a, "ld a, [hl-]", [(set A, (load HL)), (set HL, (sub HL, 1))]>;
    }
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
    def INCdHL : I8<0x34, "inc [hl]", [(store HL, (add (load HL), 1))]>;
    def DECdHL : I8<0x35, "dec [hl]", [(store HL, (sub (load HL), 1))]>;
}

def LDri : SM83Inst8<(outs GR8:$dst), (ins i8imm:$imm), "ld $dst, $imm",
                     [(set GR8:$dst, i8imm:$imm)]> {
    bits<3> dst;
    bits<8> imm;

    let Inst{7-6} = 0b00;
    let Inst{5-3} = dst;
    let Inst{2-0} = 0b110;

    let Inst{15-8} = imm;
}

let mayStore = 1, Uses = [HL] in
def LDdHLi : SM83Inst16<(outs), (ins i8imm:$imm), "ld [hl], $imm",
                       [(store HL, i8imm:$imm)]> {
    bits<8> imm;

    let Inst{7-0} = 0x36;
    let Inst{15-8} = imm;
}

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
        def i : SM83Inst16<(outs), (ins i8imm:$imm), opstr # " a, $imm",
                           [(set A, (op A, i8imm:$imm))]> {
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

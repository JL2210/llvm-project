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

class ALU_r<string opstr, bits<3> opc, SDPatternOperator op>
  : SM83Inst8<(outs), (ins GR8:$src), opstr # " a, $src",
              [(set A, (op A, GR8:$src))]> {
    bits<3> src;

    let Inst{7-6} = 0b10;
    let Inst{5-3} = opc;
    let Inst{2-0} = src;
}

class ALU_i<string opstr, bits<3> opc, SDPatternOperator op>
  : SM83Inst16<(outs), (ins i8imm:$imm), opstr # " a, $imm",
               [(set A, (op A, i8imm:$imm))]> {
    bits<8> imm;

    let Inst{7-6}  = 0b11;
    let Inst{5-3}  = opc;
    let Inst{2-0}  = 0b110;
    let Inst{15-8} = imm;
}

let mayLoad = 1 in
class ALU_dHL<string opstr, bits<3> opc, SDPatternOperator op>
  : SM83Inst8<(outs), (ins), opstr # " a, [hl]",
              [(set A, (op A, (load HL)))]> {
    let Inst{7-6} = 0b10;
    let Inst{5-3} = opc;
    let Inst{2-0} = 0b110;
}

multiclass mALU<string opstr, bits<3> opc, list<SM83Reg> uses,
                SDPatternOperator op = null_frag> {
    let Uses = uses in {
        def r : ALU_r<opstr, opc, op>;
        def i : ALU_i<opstr, opc, op>;
    }

    let Uses = !listconcat(uses, [HL]), mayLoad = 1 in
    def dHL : ALU_dHL<opstr, opc, op>;
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

def LDddnn : SM83Inst24<(outs GR16:$dst), (ins i16imm:$imm),
             "ld $dst, $imm", [(set GR16:$dst, i16imm:$imm)]> {
    bits<2> dst;
    bits<16> imm;

    let Inst{7-6} = 0b00;
    let Inst{5-4} = dst; // Destination
    let Inst{3-0} = 0b0001;

    let Inst{23-8} = imm; // Operand
}

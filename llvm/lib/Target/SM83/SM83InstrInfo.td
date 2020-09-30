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

multiclass ALU_ri<string opstr, bits<3> opc, SDPatternOperator op> {
  def r : ALU_r<opc,
                (outs), (ins GR8:$src),
		opstr # " a, $src",
                [(set A, (op A, GR8:$src))]>;
  def i : ALU_i<opc,
                (outs), (ins i8imm:$imm),
                opstr # " a, $imm",
                [(set A, (op A, i8imm:$imm))]>;
}

defm ADD : ALU_ri<"add", 0b000, add>;
defm SUB : ALU_ri<"sub", 0b010, sub>;
defm AND : ALU_ri<"and", 0b100, and>;
defm XOR : ALU_ri<"xor", 0b101, xor>;
defm OR  : ALU_ri<"or" , 0b110, or >;

def LDddnn : SM83Inst24<(outs AR16:$dst),
                        (ins i16imm:$imm),
                        "ld $dst, $imm",
                        [(set AR16:$dst, i16imm:$imm)]>
{
  bits<2> dst;
  bits<16> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = dst; // Destination
  let Inst{3-0} = 0b0001;

  let Inst{8-23} = imm; // Operand
}
                       


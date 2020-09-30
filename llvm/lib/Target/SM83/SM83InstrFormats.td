//===-- SM83InstrInfo.td - Target Description for SM83 -----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// generic SM83 instruction
class SM83Instruction<dag outs, dag ins, string asmstr, list<dag> pattern = []>
  : Instruction
{
  let Namespace = "SM83";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

class SM83Inst8<dag outs, dag ins, string asmstr, list<dag> pattern = []>
  : SM83Instruction<outs, ins, asmstr, pattern>
{
  field bits<8> Inst;

  let Size = 1;
}

class SM83Inst16<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Instruction<outs, ins, asmstr, pattern>
{
  field bits<16> Inst;

  let Size = 2;
}

class SM83Inst24<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Instruction<outs, ins, asmstr, pattern>
{
  field bits<24> Inst;
 
  let Size = 3;
}

let Uses = [A], Defs = [A, FLAGS] in {
  class ALU_r<bits<3> op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : SM83Inst8<outs, ins, asmstr, pattern>
  {
      bits<3> src;

      let Inst{7-6} = 0b10;
      let Inst{5-3} = op;
      let Inst{2-0} = src;
  }

  class ALU_i<bits<3> op, dag outs, dag ins, string asmstr, list<dag> pattern>  
    : SM83Inst16<outs, ins, asmstr, pattern>
  {
      bits<8> imm;

      let Inst{7-6}  = 0b11;
      let Inst{5-3}  = op;
      let Inst{2-0}  = 0b110;
      let Inst{15-8} = imm;
  }
}

class PrefixCB<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern>
{
    let Inst{7-0} = 0xCB;
}

class SHRO_r<bits<3> op, dag outs, dag ins, string asmstr, list<dag> pattern>
  : PrefixCB<outs, ins, asmstr, pattern>
{
    bits<3> dst;

    let Inst{15-14} = 0b00;
    let Inst{13-11} = op;
    let Inst{10-8}  = dst;
}

class BTS_ri<bits<2> op, dag outs, dag ins, string asmstr, list<dag> pattern>
  : PrefixCB<outs, ins, asmstr, pattern>
{
    bits<3> dst;
    bits<3> idx;

    let Inst{15-14} = op;
    let Inst{13-11} = idx;
    let Inst{10-8}  = dst;
}

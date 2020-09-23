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

// arithmetic operations
class SM83ALU<bits<3> op> {
  bits<3> Value = op;
}
def ALU_ADD : SM83ALU<0b000>;
def ALU_ADC : SM83ALU<0b001>;
def ALU_SUB : SM83ALU<0b010>;
def ALU_SBC : SM83ALU<0b011>;
def ALU_AND : SM83ALU<0b100>;
def ALU_XOR : SM83ALU<0b101>;
def ALU_OR  : SM83ALU<0b110>;
def ALU_CP  : SM83ALU<0b111>;

let Uses = [A], Defs = [A, FLAGS] in {
  class ALU_r<SM83ALU op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : SM83Inst8<outs, ins, asmstr, pattern>
  {
      bits<3> src;

      let Inst{7-6} = 0b10;
      let Inst{5-3} = op.Value;
      let Inst{2-0} = src;
  }

  class ALU_i<SM83ALU op, dag outs, dag ins, string asmstr, list<dag> pattern>  
    : SM83Inst16<outs, ins, asmstr, pattern>
  {
      bits<8> imm;

      let Inst{7-6}  = 0b11;
      let Inst{5-3}  = op.Value;
      let Inst{2-0}  = 0b110;
      let Inst{15-8} = imm;
  }
}

class PrefixCB<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern>
{
    let Inst{7-0} = 0xCB;
}

// shifts and rotates
class SM83SHRO<bits<3> type> {
    bits<3> Value = type;
}
def RO_RLC  : SM83SHRO<0b000>;
def RO_RRC  : SM83SHRO<0b001>;
def RO_RL   : SM83SHRO<0b010>;
def RO_RR   : SM83SHRO<0b011>;
def SH_SLA  : SM83SHRO<0b100>;
def SH_SRA  : SM83SHRO<0b101>;
def SH_SWAP : SM83SHRO<0b110>;
def SH_SRL  : SM83SHRO<0b111>;

class SHRO_r<SM83SHRO type, dag outs, dag ins, string asmstr, list<dag> pattern>
  : PrefixCB<outs, ins, asmstr, pattern>
{
    bits<3> dst;

    let Inst{15-14} = 0b00;
    let Inst{13-11} = type.Value;
    let Inst{10-8}  = dst;
}

// bit tests and sets
class SM83BTS<bits<2> op> {
  bits<2> Value = op;
}
// 00 is shifts and rotates
def BTS_BIT : SM83BTS<0b01>;
def BTS_RES : SM83BTS<0b10>;
def BTS_SET : SM83BTS<0b11>;

class BTS_ri<SM83BTS op, dag outs, dag ins, string asmstr, list<dag> pattern>
  : PrefixCB<outs, ins, asmstr, pattern>
{
    bits<3> dst;
    bits<3> idx;

    let Inst{15-14} = op.Value;
    let Inst{13-11} = idx;
    let Inst{10-8}  = dst;
}

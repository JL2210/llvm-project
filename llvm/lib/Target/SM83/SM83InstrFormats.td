//===-- SM83InstrInfo.td - Target Description for SM83 -----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// generic SM83 instruction
let mayLoad = 0, mayStore = 0, hasSideEffects = 0 in
class SM83Instruction<dag outs, dag ins, string asmstr, list<dag> pattern>
  : Instruction {
  let Namespace = "SM83";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

let isPseudo = 1, isCodeGenOnly = 1 in
class Pseudo<dag outs, dag ins, list<dag> pattern, string asmstr = "">
  : SM83Instruction<outs, ins, asmstr, pattern> {
}

class SM83Inst8<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Instruction<outs, ins, asmstr, pattern> {
  field bits<8> Inst;

  let Size = 1;
}

class I8<bits<8> bin, string asmstr, list<dag> pattern = [], dag outs = (outs),
         dag ins = (ins)> : SM83Inst8<outs, ins, asmstr, pattern> {
  let Inst{7-0} = bin;
}

class SM83Inst16<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Instruction<outs, ins, asmstr, pattern>
{
  field bits<16> Inst;

  let Size = 2;
}

class CB<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern> {
  let Inst{7-0} = 0xcb;
}

class SM83Inst24<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Instruction<outs, ins, asmstr, pattern>
{
  field bits<24> Inst;
 
  let Size = 3;
}

class PrefixCB<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern>
{
    let Inst{7-0} = 0xCB;
}

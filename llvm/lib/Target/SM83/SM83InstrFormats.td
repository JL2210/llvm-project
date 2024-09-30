// generic SM83 instruction
def SM83Instruction<dag outs, dag ins, string asmstr, list<dag> pattern>
  : Instruction
{
  let Namespace = "SM83";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

def SM83Inst8<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Instruction<outs, ins, asmstr, pattern>
{
  field bits<8> Inst;

  let Size = 1;
}

def SM83Inst16<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Instruction<outs, ins, asmstr, pattern>
{
  field bits<16> Inst;

  let Size = 2;
}

def SM83Inst24<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Instruction<outs, ins, asmstr, pattern>
{
  field bits<24> Inst;
 
  let Size = 3;
}

//===----------------------------------------------------------------------===//
// register/register load: <|01|src|dst|>
// dst - destination - 3 bits
// src - source - 3 bits
//===----------------------------------------------------------------------===//
def LRdRs<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst8<outs, ins, asmstr, pattern>
{
  bits<3> dst;
  bits<3> src;

  let Inst{0-2} = dst;
  let Inst{3-5} = src;
}

//===----------------------------------------------------------------------===//
// register/immediate8 load: <|00|dst|110|constant|>
// dst - destination - 3 bits
// constant - constant data - 8 bits
//===----------------------------------------------------------------------===//
def LRdI8<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern>
{
  bits<3> dst;
  bits<8> constant;

  let Inst{3-5} = dst;
  let Inst{8-15} = constant;
}

//===----------------------------------------------------------------------===//
// register/A arithmetic: <|10|opc|src|>
// opc - type of operation - 3 bits
// src - source register - 3 bits
//===----------------------------------------------------------------------===//
def ARs<bits<3> opc, dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst8<outs, ins, asmstr, pattern>
{
  bits<3> src;

  let Inst{0-2} = src;
  let Inst{3-5} = opc;
}

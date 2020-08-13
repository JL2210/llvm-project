// generic SM83 instruction
def SM83Instruction<dag outs, dag ins, string asmstr, list<dag> pattern = []>
  : Instruction
{
  let Namespace = "SM83";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

def SM83Inst8<dag outs, dag ins, string asmstr, list<dag> pattern = []>
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

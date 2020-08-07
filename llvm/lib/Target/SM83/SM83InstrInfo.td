include "llvm/Target/Target.td"

def LDddnn : SM83Inst24<(outs GR32:$dst),
                        (ins i16imm:$src),
                        "ld $dst, $src",
                        [(set i16:$dst, i16imm:$src)]>
{
  bits<2> dst;
  bits<16> src;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = dst; // Destination
  let Inst{3-0} = 0b0001;

  let Inst{8-23} = imm; // Operand
}

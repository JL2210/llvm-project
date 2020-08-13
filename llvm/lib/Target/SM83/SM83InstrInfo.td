include "llvm/Target/Target.td"

def LDddnn : SM83Inst24<(outs AR16:$dst),
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

def INCss : SM83Inst24<(outs AR16:$dst),
                       

//===----------------------------------------------------------------------===//
// register pair increment/decrement: <|00|rp|d|011|>
// rp - reg pair - 2 bits
// d - decrement - 1 bit
//===----------------------------------------------------------------------===//
def A_Rp<bit d, dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst8<outs, ins, asmstr, pattern>
{
  bits<2> rp;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = rp;
  let Inst{3} = d;
  let Inst{2-0} = 0b011;
}

//===----------------------------------------------------------------------===//
// register inc/dec: <|00|reg|10|d|>
// reg - source/destination register - 3 bits
// d - dec - 1 bit
//===----------------------------------------------------------------------===//
def A_Rr<bit d, dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst8<outs, ins, asmstr, pattern>
{
  bits<3> reg;

  let Inst{7-6} = 0b00;
  let Inst{5-3} = reg;
  let Inst{2-1} = 0b10;
  let Inst{0} = d;
}

//===----------------------------------------------------------------------===//
// register/immediate load: <|00|dst|110|>,imm
// dst - destination - 3 bits
// imm - immediate - 8 bits
//===----------------------------------------------------------------------===//
def L_Rd_I8<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern>
{
  bits<3> dst;
  bits<8> imm;

  let Inst{7-6} = 0b00;
  let Inst{5-3} = dst;
  let Inst{2-0} = 0b110;

  let Inst{8-15} = imm;
}

//===----------------------------------------------------------------------===//
// HL/register pair arithmetic: <|00|rp|1001|>
// rp - reg pair - 2 bits
//===----------------------------------------------------------------------===//
def A_HL_Rp<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst8<outs, ins, asmstr, pattern>
{
  bits<2> rp;

  let Inst{7-6} = 0b00;
  let Inst{5-4} = rp;
  let Inst{3-0} = 0b1001;
}

//===----------------------------------------------------------------------===//
// conditional relative jump: <|001|cc|000|>,imm
// cc - condition - 2 bits
// imm - signed immediate - 8 bits
//===----------------------------------------------------------------------===//
def JR_CC<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern>
{
  bits<2> cc;
  bits<8> imm;

  let Inst{7-5} = 0b001;
  let Inst{4-3} = cc;
  let Inst{2-0} = 0b000;

  let Inst{8-15} = imm; 
}

//===----------------------------------------------------------------------===//
// register/register load: <|01|src|dst|>
// dst - destination - 3 bits
// src - source - 3 bits
//===----------------------------------------------------------------------===//
def L_Rd_Rs<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst8<outs, ins, asmstr, pattern>
{
  bits<3> dst;
  bits<3> src;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = src;
  let Inst{2-0} = dst;
}

//===----------------------------------------------------------------------===//
// A/register arithmetic: <|10|opc|src|>
// opc - type of operation - 3 bits
// src - source - 3 bits
//===----------------------------------------------------------------------===//
def A_A_Rs<bits<3> opc, dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst8<outs, ins, asmstr, pattern>
{
  bits<3> src;

  let Inst{7-6} = 0b10;
  let Inst{5-3} = opc;
  let Inst{2-0} = src;
}

//===----------------------------------------------------------------------===//
// A/immediate arithmetic: <|11|opc|110|>,imm
// opc - operation - 3 bits
// imm - immediate - 8 bits
//===----------------------------------------------------------------------===//
def A_A_I8<bits<3> opc, dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern>
{
  bits<8> imm;

  let Inst{7-6} = 0b11;
  let Inst{5-3} = opc;
  let Inst{2-0} = 0b110;

  let Inst{8-15} = imm;
}

//===----------------------------------------------------------------------===//
// software reset: <|11|rst|111|>
// rst - jump location - 3 bits
//===----------------------------------------------------------------------===//
def R_I3<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst8<outs, ins, asmstr, pattern>
{
  bits<3> rst;

  let Inst{7-6} = 0b11;
  let Inst{5-3} = rst;
  let Inst{2-0} = 0b111;
}

//===----------------------------------------------------------------------===//
// CB prefix: <|11001011|>
//===----------------------------------------------------------------------===//
def PrefixCB<dag outs, dag ins, string asmstr, list<dag> pattern>
  : SM83Inst16<outs, ins, asmstr, pattern>
{
  let Inst{7-0} = 0xCB;
}

//===----------------------------------------------------------------------===//
// register bit operation: CB/<|00|opc|reg|>
// opc - operation - 3 bits
// reg - source/destination - 3 bits
//===----------------------------------------------------------------------===//
def B_Rr<bits<3> opc, dag outs, dag ins, string asmstr, list<dag> pattern>
  : PrefixCB<outs, ins, asmstr, pattern>
{
  bits<3> reg;

  let Inst{15-14} = 0b00;
  let Inst{13-11} = opc;
  let Inst{10-8} = reg;
}

//===----------------------------------------------------------------------===//
// register bit test/reset/set: CB/<|op|imm|reg|>
// op - operation - 2 bits
// imm - immediate - 3 bits 
// reg - source/destination - 3 bits
//===----------------------------------------------------------------------===//
def B_I3_Rr<bits<2> op, dag outs, dag ins, string asmstr, list<dag> pattern>
  : PrefixCB<outs, ins, asmstr, pattern>
{
  bits<3> imm;
  bits<3> reg;

  let Inst{15-14} = op;
  let Inst{13-11} = imm;
  let Inst{10-8} = reg;
}

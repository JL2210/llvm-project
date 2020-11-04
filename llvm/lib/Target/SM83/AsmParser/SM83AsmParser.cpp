//===-- SM83AsmParser.cpp - Parse SM83 assembly to MCInst instructions ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SM83MCTargetDesc.h"
#include "MCTargetDesc/SM83BaseInfo.h"
#include "TargetInfo/SM83TargetInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

namespace {
struct SM83Operand;

class SM83AsmParser : public MCTargetAsmParser {
  SMLoc getLoc() const { return getParser().getTok().getLoc(); }

  bool generateImmOutOfRangeError(OperandVector &Operands, uint64_t ErrorInfo,
                                  int Lower, int Upper, Twine Msg);

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;
  OperandMatchResultTy tryParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool ParseDirective(AsmToken DirectiveID) override;

// Auto-generated instruction matching functions
#define GET_ASSEMBLER_HEADER
#include "SM83GenAsmMatcher.inc"

  OperandMatchResultTy parseImmediate(OperandVector &Operands);
  OperandMatchResultTy parseRegister(OperandVector &Operands);
  OperandMatchResultTy parseMemOpBaseReg(OperandVector &Operands);

  bool parseOperand(OperandVector &Operands);

public:
  enum SM83MatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "SM83GenAsmMatcher.inc"
#undef GET_OPERAND_DIAGNOSTIC_TYPES
  };

  SM83AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                 const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, STI, MII) {
    Parser.addAliasForDirective("db", ".byte");
    Parser.addAliasForDirective("dw", ".2byte");
    Parser.addAliasForDirective("dl", ".4byte");
    Parser.addAliasForDirective("ds", ".skip");
    setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }
};

/// SM83Operand - Instances of this class represent a parsed machine
/// instruction
struct SM83Operand : public MCParsedAsmOperand {

  enum KindTy {
    Token,
    Register,
    Immediate,
  } Kind;

  struct RegOp {
    unsigned RegNum;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  SMLoc StartLoc, EndLoc;
  union {
    StringRef Tok;
    RegOp Reg;
    ImmOp Imm;
  };

  SM83Operand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}

public:
  SM83Operand(const SM83Operand &o) : MCParsedAsmOperand() {
    Kind = o.Kind;
    StartLoc = o.StartLoc;
    EndLoc = o.EndLoc;
    switch (Kind) {
    case Register:
      Reg = o.Reg;
      break;
    case Immediate:
      Imm = o.Imm;
      break;
    case Token:
      Tok = o.Tok;
      break;
    }
  }

  bool isToken() const override { return Kind == Token; }
  bool isReg() const override { return Kind == Register; }
  bool isImm() const override { return Kind == Immediate; }
  bool isMem() const override { return false; }

  bool isConstantImm() const {
    return isImm() && dyn_cast<MCConstantExpr>(getImm());
  }

  int64_t getConstantImm() const {
    const MCExpr *Val = getImm();
    return static_cast<const MCConstantExpr *>(Val)->getValue();
  }

  bool isCondition() const {
    // TODO: FIXME: terrible hack
    if(isReg() && getReg() == SM83::C)
      return true;

    if(!isImm())
      return false;

    auto SVal = static_cast<const MCSymbolRefExpr *>(getImm());
    if (!SVal || SVal->getKind() != MCSymbolRefExpr::VK_None)
      return false;

    StringRef Str = SVal->getSymbol().getName();
    if(Str != "nz" && Str != "z" && Str != "nc" && Str != "c")
      return false;

    return true;
  }

  bool isUImm3() const {
    return isConstantImm() && isUInt<3>(getConstantImm());
  }

  bool isRSTVec() const {
    return isConstantImm() && isShiftedUInt<3, 3>(getConstantImm());
  }

  bool isImm8() const {
    if(!isConstantImm())
      return false;
    uint64_t imm = getConstantImm();
    return isInt<8>(imm) || isUInt<8>(imm);
  }

  bool isImm16() const {
    if(!isConstantImm())
      return false;
    uint64_t imm = getConstantImm();
    return isInt<16>(imm) || isUInt<16>(imm);
  }

  bool isDirect16() const {
    return isConstantImm() && isUInt<16>(getConstantImm());
  }

  bool isDirect8() const {
    if(!isConstantImm())
      return false;
    uint64_t imm = getConstantImm()
    return imm & ~UINT64_C(0xff) == 0xff00;
  }

  bool isRel8() const {
    return isConstantImm() && isInt<8>(getConstantImm());
  }

  /// getStartLoc - Gets location of the first token of this operand
  SMLoc getStartLoc() const override { return StartLoc; }
  /// getEndLoc - Gets location of the last token of this operand
  SMLoc getEndLoc() const override { return EndLoc; }

  unsigned getReg() const override {
    assert(Kind == Register && "Invalid type access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert(Kind == Immediate && "Invalid type access!");
    return Imm.Val;
  }

  StringRef getToken() const {
    assert(Kind == Token && "Invalid type access!");
    return Tok;
  }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case Immediate:
      OS << *getImm();
      break;
    case Register:
      OS << "<register x";
      OS << getReg() << ">";
      break;
    case Token:
      OS << "'" << getToken() << "'";
      break;
    }
  }

  static std::unique_ptr<SM83Operand> createToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<SM83Operand>(Token);
    Op->Tok = Str;
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<SM83Operand> createReg(unsigned RegNo, SMLoc S,
                                                 SMLoc E) {
    auto Op = std::make_unique<SM83Operand>(Register);
    Op->Reg.RegNum = RegNo;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<SM83Operand> createImm(const MCExpr *Val, SMLoc S,
                                                 SMLoc E) {
    auto Op = std::make_unique<SM83Operand>(Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    assert(Expr && "Expr shouldn't be null!");
    if (auto *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  // Used by the TableGen Code
  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addConditionOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    StringRef Name;
    // TODO: FIXME: terrible hack
    // this might be matched as a register
    if(isReg() && getReg() == SM83::C) {
      Name = "c";
    } else {
      auto SE = static_cast<const MCSymbolRefExpr *>(getImm());
      Name = SE->getSymbol().getName();
    }

    unsigned Imm;
    if(Name == "nz")
      Imm = SM83Condition::NZ;
    else if(Name == "z")
      Imm = SM83Condition::Z;
    else if(Name == "nc")
      Imm = SM83Condition::NC;
    else if(Name == "c")
      Imm = SM83Condition::C;
    else
      llvm_unreachable("Condition must contain only nz, z, nc, or c");
    Inst.addOperand(MCOperand::createImm(Imm));
  }
};
} // end anonymous namespace.

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "SM83GenAsmMatcher.inc"

bool SM83AsmParser::generateImmOutOfRangeError(
    OperandVector &Operands, uint64_t ErrorInfo, int Lower, int Upper,
    Twine Msg = "immediate must be an integer in the range") {
  SMLoc ErrorLoc = static_cast<SM83Operand &>(*Operands[ErrorInfo]).getStartLoc();
  return Error(ErrorLoc, Msg + " [" + Twine(Lower) + ", " + Twine(Upper) + "]");
}

bool SM83AsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;

  switch (MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm)) {
  default:
    break;
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_MissingFeature:
    return Error(IDLoc, "instruction use requires an option to be enabled");
  case Match_MnemonicFail:
    return Error(IDLoc, "unrecognized instruction mnemonic");
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0U) {
      if (ErrorInfo >= Operands.size())
        return Error(ErrorLoc, "too few operands for instruction");

      ErrorLoc = static_cast<SM83Operand &>(*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }
    return Error(ErrorLoc, "invalid operand for instruction");
  }
  case Match_InvalidUImm3:
    return generateImmOutOfRangeError(Operands, ErrorInfo, 0, (1 << 3) - 1);
  case Match_InvalidRSTVec:
    return generateImmOutOfRangeError(
           Operands, ErrorInfo, 0, (1 << 6) - 8,
           "immediate must be a multiple of 8 in the range");
  case Match_InvalidImm8:
    return generateImmOutOfRangeError(Operands, ErrorInfo, -(1 << 7), (1 << 8) - 1);
  case Match_InvalidImm16:
    return generateImmOutOfRangeError(Operands, ErrorInfo, -(1 << 15), (1 << 16) - 1);
  case Match_InvalidDirect16:
    return generateImmOutOfRangeError(Operands, ErrorInfo, 0, (1 << 16) - 1);
  case Match_InvalidRel8:
    return generateImmOutOfRangeError(Operands, ErrorInfo, -(1 << 7), (1 << 7) - 1);
  case Match_InvalidCondition: {
    SMLoc ErrLoc = static_cast<SM83Operand &>(*Operands[ErrorInfo]).getStartLoc();
    return Error(ErrLoc, "operand must be one of nz, z, nc, or c");
  }
  }

  llvm_unreachable("Unknown match type detected!");
}

bool SM83AsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                  SMLoc &EndLoc) {
  if (tryParseRegister(RegNo, StartLoc, EndLoc) != MatchOperand_Success)
    return Error(StartLoc, "invalid register name");
  return false;
}

OperandMatchResultTy SM83AsmParser::tryParseRegister(unsigned &RegNo,
                                                     SMLoc &StartLoc,
                                                     SMLoc &EndLoc) {
  const AsmToken &Tok = getParser().getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  StringRef Name = getLexer().getTok().getIdentifier();

  if (!MatchRegisterName(Name)) {
    return MatchOperand_NoMatch;
  }
  getParser().Lex();  // Eat identifier token.
  return MatchOperand_Success;
}

OperandMatchResultTy SM83AsmParser::parseRegister(OperandVector &Operands) {
  SMLoc S = getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  switch (getLexer().getKind()) {
  default:
    return MatchOperand_NoMatch;
  case AsmToken::Identifier:
    StringRef Name = getLexer().getTok().getIdentifier();
    unsigned RegNo = MatchRegisterName(Name);
    if (RegNo == 0)
      return MatchOperand_NoMatch;
    getLexer().Lex();
    Operands.push_back(SM83Operand::createReg(RegNo, S, E));
  }
  return MatchOperand_Success;
}

OperandMatchResultTy SM83AsmParser::parseImmediate(OperandVector &Operands) {
  SMLoc S = getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);
  const MCExpr *Res;

  switch (getLexer().getKind()) {
  default:
    return MatchOperand_NoMatch;
  case AsmToken::LParen:
  case AsmToken::Minus:
  case AsmToken::Plus:
  case AsmToken::Integer:
  case AsmToken::String:
    if (getParser().parseExpression(Res))
      return MatchOperand_ParseFail;
    break;
  case AsmToken::Identifier: {
    StringRef Identifier;
    if (getParser().parseIdentifier(Identifier))
      return MatchOperand_ParseFail;
    MCSymbol *Sym = getContext().getOrCreateSymbol(Identifier);
    Res = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, getContext());
    break;
  }
  }

  Operands.push_back(SM83Operand::createImm(Res, S, E));
  return MatchOperand_Success;
}

OperandMatchResultTy
SM83AsmParser::parseMemOpBaseReg(OperandVector &Operands) {
  if (getLexer().isNot(AsmToken::LBrac)) {
    Error(getLoc(), "expected '['");
    return MatchOperand_ParseFail;
  }

  getParser().Lex(); // Eat '['
  Operands.push_back(SM83Operand::createToken("[", getLoc()));

  if (parseRegister(Operands) != MatchOperand_Success) {
    Error(getLoc(), "expected register");
    return MatchOperand_ParseFail;
  }

  if (getLexer().is(AsmToken::Plus) ||
      getLexer().is(AsmToken::Minus)) {
    auto str = getTok().getString();
    getParser().Lex();
    Operands.push_back(SM83Operand::createToken(str, getLoc()));
  }

  if (getLexer().isNot(AsmToken::RBrac)) {
    Error(getLoc(), "expected ']'");
    return MatchOperand_ParseFail;
  }

  getParser().Lex(); // Eat ']'
  Operands.push_back(SM83Operand::createToken("]", getLoc()));

  return MatchOperand_Success;
}

/// Looks at a token type and creates the relevant operand
/// from this information, adding to Operands.
/// If operand was parsed, returns false, else true.
bool SM83AsmParser::parseOperand(OperandVector &Operands) {
  // Attempt to parse token as register
  if (parseRegister(Operands) == MatchOperand_Success)
    return false;

  // Attempt to parse token as an immediate
  if (parseImmediate(Operands) == MatchOperand_Success) {
    // Parse memory base register if present
    if (getLexer().is(AsmToken::LBrac))
      return parseMemOpBaseReg(Operands) != MatchOperand_Success;
    return false;
  }

  // Finally we have exhausted all options and must declare defeat.
  Error(getLoc(), "unknown operand");
  return true;
}

bool SM83AsmParser::ParseInstruction(ParseInstructionInfo &Info,
                                      StringRef Name, SMLoc NameLoc,
                                      OperandVector &Operands) {
  // First operand is token for instruction
  Operands.push_back(SM83Operand::createToken(Name, NameLoc));

  // If there are no more operands, then finish
  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  // Parse first operand
  if (parseOperand(Operands))
    return true;

  // Parse until end of statement, consuming commas between operands
  while (getLexer().is(AsmToken::Comma)) {
    // Consume comma token
    getLexer().Lex();

    // Parse next operand
    if (parseOperand(Operands))
      return true;
  }

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    getParser().eatToEndOfStatement();
    return Error(Loc, "unexpected token");
  }

  getParser().Lex(); // Consume the EndOfStatement.
  return false;
}

bool SM83AsmParser::ParseDirective(AsmToken DirectiveID) { return true; }

extern "C" void LLVMInitializeSM83AsmParser() {
  RegisterMCAsmParser<SM83AsmParser> X(getTheSM83Target());
}

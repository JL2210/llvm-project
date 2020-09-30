//===-- SM83AsmBackend.cpp - SM83 Assembler Backend -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SM83MCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class SM83AsmBackend : public MCAsmBackend {

public:
  SM83AsmBackend() : MCAsmBackend(support::little) {}
  ~SM83AsmBackend() override {}

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target,
                  MutableArrayRef<char> Data, uint64_t Value,
                  bool IsResolved, const MCSubtargetInfo *STI) const override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override;

  unsigned getNumFixupKinds() const override {
    /* TODO */
    return 1;
  }

  bool mayNeedRelaxation(const MCInst &Inst,
                         const MCSubtargetInfo &STI) const override {
    return false;
  }

  void relaxInstruction(MCInst &Inst,
                        const MCSubtargetInfo &STI) const override {
    report_fatal_error("SM83AsmBackend::relaxInstruction() unimplemented");
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count) const override;
}; // class SM83AsmBackend

bool SM83AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count) const {
  // nop is 0
  OS.write_zeros(Count);
  return true;
}

void SM83AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                const MCValue &Target,
                                MutableArrayRef<char> Data, uint64_t Value,
                                bool IsResolved, const MCSubtargetInfo *STI) const {
  /* TODO */
}

bool SM83AsmBackend::fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                                          const MCRelaxableFragment *DF,
                                          const MCAsmLayout &Layout) const {
  return false;
}

std::unique_ptr<MCObjectTargetWriter>
SM83AsmBackend::createObjectTargetWriter() const {
  return createSM83ELFObjectWriter();
}

} // end anonymous namespace

MCAsmBackend *llvm::createSM83AsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  return new SM83AsmBackend();
}

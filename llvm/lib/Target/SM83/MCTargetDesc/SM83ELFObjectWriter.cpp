//===-- SM83ELFObjectWriter.cpp - SM83 ELF Writer -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SM83MCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class SM83ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  SM83ELFObjectWriter()
    : MCELFObjectTargetWriter(/*Is64Bit=*/false, /*OSABI=*/0, ELF::EM_SM83,
                              /*HasRelocationAddend=*/true) {}

  ~SM83ELFObjectWriter() override { }

protected:
  unsigned getRelocType(MCContext &CTX, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
}

unsigned SM83ELFObjectWriter::getRelocType(MCContext &CTX,
                                            const MCValue &Target,
                                            const MCFixup &Fixup,
                                            bool IsPCRel) const {
  report_fatal_error("invalid fixup kind!");
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createSM83ELFObjectWriter() {
  return std::make_unique<SM83ELFObjectWriter>();
}

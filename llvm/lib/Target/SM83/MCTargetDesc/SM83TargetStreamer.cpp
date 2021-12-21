//===- SM83TargetStreamer.cpp - SM83TargetStreamer class --*- C++ -*-------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SM83TargetStreamer class.
//
//===----------------------------------------------------------------------===//

#include "SM83TargetStreamer.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

SM83TargetStreamer::SM83TargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S) {}

SM83TargetAsmStreamer::SM83TargetAsmStreamer(MCStreamer &S,
                                             formatted_raw_ostream &OS)
    : SM83TargetStreamer(S), MAI(S.getContext().getAsmInfo()), OS(OS) {}

void SM83TargetAsmStreamer::emitAlign(Align Alignment) {
  if(Log2(Alignment))
    OS << "\talign " << Log2(Alignment) << '\n';
}

void SM83TargetAsmStreamer::emitBlock(uint64_t NumBytes) {
  if (NumBytes)
    OS << "\tds\t" << NumBytes << '\n';
}

void SM83TargetAsmStreamer::emitGlobal(MCSymbol *Symbol) {
  Symbol->print(OS, MAI);
  OS << "::\n";
}

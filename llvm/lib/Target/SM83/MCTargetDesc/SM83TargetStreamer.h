//==-- SM83TargetStreamer.h - SM83 Target Streamer ---------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file declares SM83-specific target streamer classes.
/// These are for implementing support for target-specific assembly directives.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83TARGETSTREAMER_H
#define LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83TARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/Alignment.h"

namespace llvm {
class formatted_raw_ostream;

class SM83TargetStreamer : public MCTargetStreamer {
public:
  explicit SM83TargetStreamer(MCStreamer &S);

  virtual void emitAlign(Align Alignment) = 0;
  virtual void emitBlock(uint64_t NumBytes) = 0;
  virtual void emitGlobal(MCSymbol *Symbol) = 0;
};

class SM83TargetAsmStreamer final : public SM83TargetStreamer {
  const MCAsmInfo *MAI;
  formatted_raw_ostream &OS;

public:
  SM83TargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);

  void emitAlign(Align Alignment) override;
  void emitBlock(uint64_t NumBytes) override;
  void emitGlobal(MCSymbol *Symbol) override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SM83_MCTARGETDESC_SM83TARGETSTREAMER_H

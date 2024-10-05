//===-- SM83MCAsmInfo.cpp - SM83 Asm properties ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the SM83AsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "SM83MCAsmInfo.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/TargetParser/Triple.h"
using namespace llvm;

void SM83MCAsmInfo::anchor() {}

SM83MCAsmInfo::SM83MCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
  CodePointerSize = 2;
  CalleeSaveStackSlotSize = 2;
  AlignmentIsInBytes = false;
  IsLittleEndian = true;
  MaxInstLength = 3;
  UseIntegratedAssembler = false;
  PreserveAsmComments = true;
  SeparatorString = "\n";
  CommentString = ";";
  AllowAdditionalComments = false;
  SupportsQuotedNames = false;
  HasDotTypeDotSizeDirective = false;
  HasSingleParameterDotFile = false;
  HiddenVisibilityAttr = HiddenDeclarationVisibilityAttr =
      ProtectedVisibilityAttr = MCSA_Invalid;
  GlobalDirective = "\tEXPORT\t";
  PrivateGlobalPrefix = "";
  PrivateLabelPrefix = ".";
  WeakDirective = "";
  InlineAsmStart = InlineAsmEnd = "";
  AllowAtInName = true;
  SupportsQuotedNames = false;
  UseDotAlignForAlignment = true; // see sm83 hack in mcasmstreamer
  AlignDirective = "\tALIGN\t";
  ZeroDirective = "\tds\t";
  AsciiDirective = "\tdb\t";
  AscizDirective = nullptr;
  ByteListDirective = Data8bitsDirective = "\tdb\t";
  Data16bitsDirective = "\tdw\t";
  Data32bitsDirective = "\tdl\t";
  Data64bitsDirective = nullptr;
}

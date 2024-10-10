//===--- SM83.cpp - Implement SM83 target feature support -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements SM83 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "SM83.h"
#include "clang/Basic/MacroBuilder.h"

using namespace clang;
using namespace clang::targets;

void SM83TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
    Builder.defineMacro("__naked", "__attribute__((__naked__))");
    Builder.defineMacro("__sfr", "__attribute__((__address_space__(1))) char");
}

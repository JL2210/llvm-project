//=- SM83Combine.td - Define SM83 Combine Rules --------------*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/GlobalISel/Combine.td"

def SM83PreLegalizerCombiner: GICombiner<
  "SM83PreLegalizerCombinerImpl", [all_combines]> {
}

def SM83O0PreLegalizerCombiner: GICombiner<
  "SM83O0PreLegalizerCombinerImpl", [optnone_combines]> {
}

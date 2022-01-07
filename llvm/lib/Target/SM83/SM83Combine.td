//=- SM83Combine.td - Define SM83 Combine Rules --------------*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/GlobalISel/Combine.td"

def SM83CombinerHelper: GICombinerHelper<
  "SM83GenCombinerHelper", [all_combines]> {
  let DisableRuleOption = "sm83combiner-disable-rule";
  let StateClass = "SM83CombinerHelperState";
  let AdditionalArguments = [];
}

def SM83O0PreLegalizerCombinerHelper: GICombinerHelper<
  "SM83GenO0PreLegalizerCombinerHelper", [optnone_combines]> {
  let DisableRuleOption = "sm83O0prelegalizercombiner-disable-rule";
  let StateClass = "SM83O0PreLegalizerCombinerHelperState";
  let AdditionalArguments = [];
}

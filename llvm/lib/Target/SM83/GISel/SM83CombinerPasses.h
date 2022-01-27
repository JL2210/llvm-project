#ifndef LLVM_LIB_TARGET_SM83_GISEL_SM83COMBINERPASSES_H
#define LLVM_LIB_TARGET_SM83_GISEL_SM83COMBINERPASSES_H

namespace llvm {

class FunctionPass;
class PassRegistry;

FunctionPass *createSM83Combiner();
FunctionPass *createSM83O0PreLegalizerCombiner();

void initializeSM83CombinerPass(PassRegistry &);
void initializeSM83O0PreLegalizerCombinerPass(PassRegistry &);

} // namespace llvm

#endif

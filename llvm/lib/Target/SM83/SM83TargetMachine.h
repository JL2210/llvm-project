#include "llvm/Target/TargetMachine.h"

namespace llvm {

class SM83TargetMachine final : public LLVMTargetMachine {
public:
  SM83TargetMachine(const Target &T, const Triple &TT);
}

} // end namespace llvm

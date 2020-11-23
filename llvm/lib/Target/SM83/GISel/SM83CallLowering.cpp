#include "SM83CallLowering.h"

#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/Analysis.h"

using namespace llvm;

#define DEBUG_TYPE "sm83-calllowering"

namespace {
struct IncomingArgHandler : public CallLowering::IncomingValueHandler {
  const DataLayout &DL;

public:
  IncomingArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                     CCAssignFn *AssignFn)
      : IncomingValueHandler(MIRBuilder, MRI, AssignFn),
        DL(MIRBuilder.getMF().getDataLayout()) {}

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        CCValAssign &VA) override {
    switch (VA.getLocInfo()) {
    default:
      MIRBuilder.buildCopy(ValVReg, PhysReg);
      break;
    case CCValAssign::LocInfo::SExt:
    case CCValAssign::LocInfo::ZExt:
    case CCValAssign::LocInfo::AExt: {
      auto Copy = MIRBuilder.buildCopy(LLT{VA.getLocVT()}, PhysReg);
      MIRBuilder.buildTrunc(ValVReg, Copy);
      break;
    }
    }
    MIRBuilder.getMBB().addLiveIn(PhysReg);
  }

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO) override {
    auto &MFI = MIRBuilder.getMF().getFrameInfo();
    int FI = MFI.CreateFixedObject(Size, Offset, true);
    MPO = MachinePointerInfo::getFixedStack(MIRBuilder.getMF(), FI);

    return MIRBuilder
        .buildFrameIndex(LLT::pointer(0, DL.getPointerSizeInBits(0)), FI)
        .getReg(0);
  }

  void assignValueToAddress(Register ValVReg, Register Addr, uint64_t Size,
                            MachinePointerInfo &MPO, CCValAssign &VA) override {
    MachineFunction &MF = MIRBuilder.getMF();
    auto *MMO = MF.getMachineMemOperand(
        MPO, MachineMemOperand::MOLoad | MachineMemOperand::MOInvariant, Size,
        inferAlignFromPtrInfo(MF, MPO));
    MIRBuilder.buildLoad(ValVReg, Addr, *MMO);
  }
};

struct OutgoingArgHandler : public CallLowering::OutgoingValueHandler {
  MachineInstrBuilder &MIB;

public:
  OutgoingArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                     MachineInstrBuilder &MIB, CCAssignFn *AssignFn)
      : OutgoingValueHandler(MIRBuilder, MRI, AssignFn), MIB(MIB) {}

  void assignValueToReg(Register ValVReg, Register PhysReg, CCValAssign &VA) override {
    Register ExtReg = extendRegister(ValVReg, VA);
    MIRBuilder.buildCopy(PhysReg, ExtReg);
    MIB.addUse(PhysReg, RegState::Implicit);
  }

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO) override {
    llvm_unreachable("unimplemented!");
  }

  void assignValueToAddress(Register ValVReg, Register Addr, uint64_t Size,
                            MachinePointerInfo &MPO, CCValAssign &VA) override {
    llvm_unreachable("unimplemented!");
  }
};

}

void SM83CallLowering::splitToValueTypes(
    const ArgInfo &OrigArg, SmallVectorImpl<ArgInfo> &SplitArgs,
    const DataLayout &DL, SplitArgTy PerformArgSplit) const {
  const SM83TargetLowering &TLI = *getTLI<SM83TargetLowering>();
  LLVMContext &Ctx = OrigArg.Ty->getContext();

  SmallVector<EVT, 4> SplitVTs;
  ComputeValueVTs(TLI, DL, OrigArg.Ty, SplitVTs);

  if (SplitVTs.size() == 0)
    return;

  if (SplitVTs.size() == 1) {
    // No splitting to do, but we want to replace the original type (e.g. [1 x
    // double] -> double).
    SplitArgs.emplace_back(OrigArg.Regs[0], SplitVTs[0].getTypeForEVT(Ctx),
                           OrigArg.Flags[0], OrigArg.IsFixed);
    return;
  }

  // Create one ArgInfo for each virtual register in the original ArgInfo.
  assert(OrigArg.Regs.size() == SplitVTs.size() && "Regs / types mismatch");

  for (unsigned i = 0, e = SplitVTs.size(); i < e; ++i) {
    Type *SplitTy = SplitVTs[i].getTypeForEVT(Ctx);
    SplitArgs.emplace_back(OrigArg.Regs[i], SplitTy, OrigArg.Flags[0],
                           OrigArg.IsFixed);
  }

  SplitArgs.back().Flags[0].setInConsecutiveRegsLast();
}

bool SM83CallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                   const Value *Val,
                                   ArrayRef<Register> VRegs) const {
  auto MIB = MIRBuilder.buildInstrNoInsert(SM83::RET);
  assert(((Val && !VRegs.empty()) || (!Val && VRegs.empty())) &&
         "Return value without a vreg");

  bool Success = true;
  if (!VRegs.empty()) {
    MachineFunction &MF = MIRBuilder.getMF();
    const Function &F = MF.getFunction();

    const SM83TargetLowering &TLI = *getTLI<SM83TargetLowering>();
    CCAssignFn *AssignFn = TLI.CCAssignFnForReturn(F.getCallingConv());
    auto &DL = F.getParent()->getDataLayout();
    LLVMContext &Ctx = Val->getType()->getContext();

    SmallVector<EVT, 4> SplitEVTs;
    ComputeValueVTs(TLI, DL, Val->getType(), SplitEVTs);
    assert(VRegs.size() == SplitEVTs.size() &&
           "For each split Type there should be exactly one VReg.");

    SmallVector<ArgInfo, 8> SplitArgs;

    for (unsigned i = 0; i < SplitEVTs.size(); ++i) {
      if (TLI.getNumRegistersForCallingConv(Ctx, F.getCallingConv(), SplitEVTs[i]) > 1) {
        LLVM_DEBUG(dbgs() << "Can't handle extended arg types which need split");
        return false;
      }

      Register CurVReg = VRegs[i];
      ArgInfo CurArgInfo = ArgInfo{CurVReg, SplitEVTs[i].getTypeForEVT(Ctx)};
      setArgFlags(CurArgInfo, AttributeList::ReturnIndex, DL, F);
      splitToValueTypes(CurArgInfo, SplitArgs, DL);
    }

    OutgoingArgHandler Handler(MIRBuilder, MF.getRegInfo(), MIB, AssignFn);
    Success = handleAssignments(MIRBuilder, SplitArgs, Handler);
  }

  MIRBuilder.insertInstr(MIB);
  return Success;
}

bool SM83CallLowering::lowerFormalArguments(
    MachineIRBuilder &MIRBuilder, const Function &F,
    ArrayRef<ArrayRef<Register>> VRegs) const {
  auto &DL = F.getParent()->getDataLayout();

  SmallVector<ArgInfo, 8> SplitArgs;
  unsigned i = 0;
  for (auto &Arg : F.args()) {
    ArgInfo OrigArg{VRegs[i], Arg.getType()};
    setArgFlags(OrigArg, i + AttributeList::FirstArgIndex, DL, F);

    splitToValueTypes(OrigArg, SplitArgs, DL);
    ++i;
  }

  const SM83TargetLowering &TLI = *getTLI<SM83TargetLowering>();
  CCAssignFn *AssignFn = TLI.CCAssignFnForCall(F.getCallingConv(), /*IsVarArg=*/false);

  IncomingArgHandler Handler(MIRBuilder, MIRBuilder.getMF().getRegInfo(), AssignFn);
  return handleAssignments(MIRBuilder, SplitArgs, Handler);
}

bool SM83CallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  return false;
}

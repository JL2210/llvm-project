#include "SM83CallLowering.h"
#include "SM83Subtarget.h"
#include "MCTargetDesc/SM83MCTargetDesc.h"

#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/Analysis.h"

using namespace llvm;

#define DEBUG_TYPE "sm83-calllowering"

namespace {

struct IncomingArgHandler : public CallLowering::IncomingValueHandler {
  const DataLayout &DL;

public:
  IncomingArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
      : IncomingValueHandler(MIRBuilder, MRI),
        DL(MIRBuilder.getMF().getDataLayout()) {}

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO, ISD::ArgFlagsTy Flags) override {
    auto &MFI = MIRBuilder.getMF().getFrameInfo();
    int FI = MFI.CreateFixedObject(Size, Offset, true);
    MPO = MachinePointerInfo::getFixedStack(MIRBuilder.getMF(), FI);

    return MIRBuilder
        .buildFrameIndex(LLT::pointer(0, DL.getPointerSizeInBits(0)), FI)
        .getReg(0);
  }

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        CCValAssign VA) override {
    markPhysRegUsed(PhysReg);
    IncomingValueHandler::assignValueToReg(ValVReg, PhysReg, VA);
  }

  void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                            MachinePointerInfo &MPO, CCValAssign &VA) override {
    MachineFunction &MF = MIRBuilder.getMF();
    auto *MMO = MF.getMachineMemOperand(
        MPO, MachineMemOperand::MOLoad | MachineMemOperand::MOInvariant, MemTy,
        inferAlignFromPtrInfo(MF, MPO));
    MIRBuilder.buildLoad(ValVReg, Addr, *MMO);
  }

  virtual void markPhysRegUsed(MCRegister PhysReg) {
    MIRBuilder.getMRI()->addLiveIn(PhysReg);
    MIRBuilder.getMBB().addLiveIn(PhysReg);
  }
}; // struct IncomingArgHandler

struct CallReturnHandler : public IncomingArgHandler {
  MachineInstrBuilder &MIB;

public:
  CallReturnHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                    MachineInstrBuilder &MIB)
      : IncomingArgHandler(MIRBuilder, MRI), MIB(MIB) {}

  void markPhysRegUsed(MCRegister PhysReg) override {
    MIB.addDef(PhysReg, RegState::Implicit);
  }
};

struct IncomingArgAssigner : public CallLowering::IncomingValueAssigner {
public:
  IncomingArgAssigner(CCAssignFn *AssignFn)
      : IncomingValueAssigner(AssignFn) {}
}; // struct IncomingArgAssigner

struct OutgoingArgAssigner : public CallLowering::OutgoingValueAssigner {
public:
  OutgoingArgAssigner(CCAssignFn *AssignFn)
      : OutgoingValueAssigner(AssignFn) {}
}; // struct OutgoingArgAssigner

struct OutgoingArgHandler : public CallLowering::OutgoingValueHandler {
  MachineInstrBuilder &MIB;
  const DataLayout &DL;
  const SM83Subtarget &STI;

public:
  OutgoingArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                     MachineInstrBuilder &MIB)
      : OutgoingValueHandler(MIRBuilder, MRI), MIB(MIB),
        DL(MIRBuilder.getMF().getDataLayout()),
        STI(MIRBuilder.getMF().getSubtarget<SM83Subtarget>()) {}

  void assignValueToReg(Register ValVReg, Register PhysReg, CCValAssign VA) override {
    Register ExtReg = extendRegister(ValVReg, VA);
    MIRBuilder.buildCopy(PhysReg, ExtReg);
    MIB.addUse(PhysReg, RegState::Implicit);
  }

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO,
                           ISD::ArgFlagsTy Flags) override {
    LLT p0 = LLT::pointer(0, DL.getPointerSizeInBits(0));
    LLT SType = LLT::scalar(DL.getPointerSizeInBits(0));
    auto SPReg =
        MIRBuilder.buildCopy(p0, STI.getRegisterInfo()->getStackRegister());

    auto OffsetReg = MIRBuilder.buildConstant(SType, Offset);

    auto AddrReg = MIRBuilder.buildPtrAdd(p0, SPReg, OffsetReg);

    MPO = MachinePointerInfo::getStack(MIRBuilder.getMF(), Offset);
    return AddrReg.getReg(0);
  }

  void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                            MachinePointerInfo &MPO, CCValAssign &VA) override {
    llvm_unreachable("assignValueToAddress unimplemented!");
  }
};

}

bool SM83CallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                   const Value *Val,
                                   ArrayRef<Register> VRegs,
                                   FunctionLoweringInfo &FLI) const {
  auto MIB = MIRBuilder.buildInstrNoInsert(SM83::RET);
  assert(((Val && !VRegs.empty()) || (!Val && VRegs.empty())) &&
         "Return value without a vreg");

  bool Success = true;
  if(!VRegs.empty()) {
    MachineFunction &MF = MIRBuilder.getMF();
    const Function &F = MF.getFunction();
    const DataLayout &DL = F.getParent()->getDataLayout();

    CallingConv::ID CC = F.getCallingConv();
    const SM83TargetLowering &TLI = *getTLI<SM83TargetLowering>();
    CCAssignFn *AssignFn = TLI.CCAssignFnForReturn(CC);

    ArgInfo OrigRetInfo(VRegs, Val->getType(), 0);
    setArgFlags(OrigRetInfo, AttributeList::ReturnIndex, DL, F);
    SmallVector<ArgInfo, 4> SplitRetInfos;

    splitToValueTypes(OrigRetInfo, SplitRetInfos, DL, CC);

    OutgoingArgAssigner Assigner(AssignFn);
    OutgoingArgHandler Handler(MIRBuilder, MF.getRegInfo(), MIB);
    Success = determineAndHandleAssignments(Handler, Assigner, SplitRetInfos, MIRBuilder, CC, F.isVarArg());
  }

  MIRBuilder.insertInstr(MIB);
  return Success;
}

bool SM83CallLowering::lowerFormalArguments(
    MachineIRBuilder &MIRBuilder, const Function &F,
    ArrayRef<ArrayRef<Register>> VRegs,
    FunctionLoweringInfo &FLI) const {
  const auto &DL = F.getParent()->getDataLayout();
  CallingConv::ID CC = F.getCallingConv();

  SmallVector<ArgInfo, 8> SplitArgs;
  unsigned i = 0;
  for (auto &Arg : F.args()) {
    ArgInfo OrigArg{VRegs[i], Arg.getType(), i};
    setArgFlags(OrigArg, i + AttributeList::FirstArgIndex, DL, F);

    splitToValueTypes(OrigArg, SplitArgs, DL, CC);
    ++i;
  }

  const SM83TargetLowering &TLI = *getTLI<SM83TargetLowering>();
  CCAssignFn *AssignFn = TLI.CCAssignFnForCall(CC, F.isVarArg());
  IncomingArgAssigner Assigner(AssignFn);
  IncomingArgHandler Handler(MIRBuilder, MIRBuilder.getMF().getRegInfo());
  return determineAndHandleAssignments(Handler, Assigner, SplitArgs, MIRBuilder, CC, F.isVarArg());
}

bool SM83CallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  MachineFunction &MF = MIRBuilder.getMF();
  const Function &F = MF.getFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const DataLayout &DL = F.getParent()->getDataLayout();
  const SM83TargetLowering &TLI = *getTLI<SM83TargetLowering>();

  if(Info.Callee.isReg()) {
    // no way
    return false;
  }

  // split the output args
  SmallVector<ArgInfo, 8> OutArgs;
  for (auto &OrigArg : Info.OrigArgs) {
    splitToValueTypes(OrigArg, OutArgs, DL, Info.CallConv);
  }

  // split the return arg
  SmallVector<ArgInfo, 8> InArgs;
  if (!Info.OrigRet.Ty->isVoidTy()) {
    splitToValueTypes(Info.OrigRet, InArgs, DL, Info.CallConv);
  }

  auto MIB = MIRBuilder.buildInstrNoInsert(SM83::CALLd16)
                       .add(Info.Callee);

  const auto &STI = MF.getSubtarget<SM83Subtarget>();
  const auto *TRI = STI.getRegisterInfo();

  CallingConv::ID CC = F.getCallingConv();
  CCAssignFn *CallAssignFn = TLI.CCAssignFnForCall(CC, F.isVarArg());
  CCAssignFn *RetAssignFn = TLI.CCAssignFnForReturn(CC);

  // args to the called function
  OutgoingArgAssigner CallAssigner(CallAssignFn);
  OutgoingArgHandler CallHandler(MIRBuilder, MRI, MIB);
  if (!determineAndHandleAssignments(CallHandler, CallAssigner, OutArgs,
                                     MIRBuilder, Info.CallConv, Info.IsVarArg))
    return false;

  const uint32_t *Mask = TRI->getCallPreservedMask(MF, Info.CallConv);
  if(Mask) MIB.addRegMask(Mask);

  MIRBuilder.insertInstr(MIB);

  // return value
  OutgoingArgAssigner RetAssigner(RetAssignFn);
  CallReturnHandler RetHandler(MIRBuilder, MRI, MIB);
  if (!determineAndHandleAssignments(RetHandler, RetAssigner, InArgs,
                                     MIRBuilder, Info.CallConv, Info.IsVarArg))
    return false;

  return true;
}

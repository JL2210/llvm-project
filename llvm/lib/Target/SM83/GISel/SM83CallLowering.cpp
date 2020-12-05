#include "SM83CallLowering.h"
#include "SM83Subtarget.h"

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
  const DataLayout &DL;
  const SM83Subtarget &STI;

public:
  OutgoingArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                     MachineInstrBuilder &MIB, CCAssignFn *AssignFn)
      : OutgoingValueHandler(MIRBuilder, MRI, AssignFn), MIB(MIB),
        DL(MIRBuilder.getMF().getDataLayout()),
        STI(MIRBuilder.getMF().getSubtarget<SM83Subtarget>()) {}

  void assignValueToReg(Register ValVReg, Register PhysReg, CCValAssign &VA) override {
    Register ExtReg = extendRegister(ValVReg, VA);
    MIRBuilder.buildCopy(PhysReg, ExtReg);
    MIB.addUse(PhysReg, RegState::Implicit);
  }

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO) override {
    LLT p0 = LLT::pointer(0, DL.getPointerSizeInBits(0));
    LLT SType = LLT::scalar(DL.getPointerSizeInBits(0));
    auto SPReg =
        MIRBuilder.buildCopy(p0, STI.getRegisterInfo()->getStackRegister());

    auto OffsetReg = MIRBuilder.buildConstant(SType, Offset);

    auto AddrReg = MIRBuilder.buildPtrAdd(p0, SPReg, OffsetReg);

    MPO = MachinePointerInfo::getStack(MIRBuilder.getMF(), Offset);
    return AddrReg.getReg(0);
  }

  void assignValueToAddress(Register ValVReg, Register Addr, uint64_t Size,
                            MachinePointerInfo &MPO, CCValAssign &VA) override {
    llvm_unreachable("assignValueToAddress unimplemented!");
  }
};

}

static void unpackRegsToOrigType(MachineIRBuilder &MIRBuilder,
                                 ArrayRef<Register> DstRegs,
                                 Register SrcReg,
                                 const CallLowering::ArgInfo &Info,
                                 LLT SrcTy,
                                 LLT PartTy) {
  assert(DstRegs.size() > 1 && "Nothing to unpack");

  if (PartTy.isVector()) {
    llvm_unreachable("cannot handle vector!");
  }

  LLT GCDTy = getGCDType(SrcTy, PartTy);
  if (GCDTy == PartTy) {
    // parts are evenly divisible
    MIRBuilder.buildUnmerge(DstRegs, SrcReg);
    return;
  } else {
    llvm_unreachable("cannot handle splits requiring extension!");
  }
}

static void packSplitRegsToOrigType(MachineIRBuilder &MIRBuilder,
                                    ArrayRef<Register> OrigRegs,
                                    ArrayRef<Register> Regs,
                                    LLT LLTy,
                                    LLT PartLLT) {
  MachineRegisterInfo &MRI = *MIRBuilder.getMRI();

  if (!LLTy.isVector() && !PartLLT.isVector()) {
    assert(OrigRegs.size() == 1);
    LLT OrigTy = MRI.getType(OrigRegs[0]);

    unsigned SrcSize = PartLLT.getSizeInBits() * Regs.size();
    if (SrcSize == OrigTy.getSizeInBits()) {
      MIRBuilder.buildMerge(OrigRegs[0], Regs);
    } else {
      auto Widened = MIRBuilder.buildMerge(LLT::scalar(SrcSize), Regs);
      MIRBuilder.buildTrunc(OrigRegs[0], Widened);
    }

    return;
  } else {
    llvm_unreachable("cannot handle vectors!");
  }
}

void SM83CallLowering::splitToValueTypes(
    const ArgInfo &OrigArg, SmallVectorImpl<ArgInfo> &SplitArgs,
    const DataLayout &DL, CallingConv::ID CallConv,
    MachineRegisterInfo &MRI, SplitArgTy PerformArgSplit) const {
  const SM83TargetLowering &TLI = *getTLI<SM83TargetLowering>();
  LLVMContext &Ctx = OrigArg.Ty->getContext();

  SmallVector<EVT, 4> SplitVTs;
  // split aggregates and arrays into those members
  ComputeValueVTs(TLI, DL, OrigArg.Ty, SplitVTs);

  // ignore empty or void arguments
  if (SplitVTs.size() == 0)
    return;

  assert(OrigArg.Regs.size() == SplitVTs.size() && "Regs / types mismatch");

  // iterate over members of aggregates and arrays and split those
  for (unsigned i = 0, e = SplitVTs.size(); i < e; ++i) {
    Register Reg = OrigArg.Regs[i];
    EVT VT = SplitVTs[i];
    Type *Ty = VT.getTypeForEVT(Ctx);
    LLT LLTy = getLLTForType(*Ty, DL);

    /* TODO: Extend? */

    // number of required registers to fit value in
    unsigned NumParts = TLI.getNumRegistersForCallingConv(Ctx, CallConv, VT);
    // types of those registers
    MVT RegVT = TLI.getRegisterTypeForCallingConv(Ctx, CallConv, VT);

    if (NumParts == 1) {
      // Replace a vector of a single element with the element itself
      SplitArgs.emplace_back(Reg, Ty, OrigArg.Flags, OrigArg.IsFixed);
      continue;
    }

    // split the argument into the required registers
    SmallVector<Register, 8> SplitRegs;
    Type *PartTy = EVT(RegVT).getTypeForEVT(Ctx);
    LLT PartLLT = getLLTForType(*PartTy, DL);

    // FIXME: Should we be reporting all of the part registers for a single
    // argument, and let handleAssignments take care of the repacking?
    for (unsigned j = 0; j < NumParts; ++j) {
      Register PartReg = MRI.createGenericVirtualRegister(PartLLT);
      SplitRegs.push_back(PartReg);
      SplitArgs.emplace_back(ArrayRef<Register>(PartReg), PartTy, OrigArg.Flags);
    }

    PerformArgSplit(SplitRegs, Reg, LLTy, PartLLT, i);
  }
}

bool SM83CallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                   const Value *Val,
                                   ArrayRef<Register> VRegs) const {
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

    ArgInfo OrigRetInfo(VRegs, Val->getType());
    setArgFlags(OrigRetInfo, AttributeList::ReturnIndex, DL, F);
    SmallVector<ArgInfo, 4> SplitRetInfos;

    splitToValueTypes(
      OrigRetInfo, SplitRetInfos, DL, CC, *MIRBuilder.getMRI(),
      [&MIRBuilder, &SplitRetInfos](ArrayRef<Register> Regs, Register SrcReg, LLT LLTy, LLT PartLLT,
          int VTSplitIdx) {
        unpackRegsToOrigType(MIRBuilder, Regs, SrcReg,
                             SplitRetInfos[VTSplitIdx],
                             LLTy, PartLLT);
      });

    CCAssignFn *AssignFn = TLI.CCAssignFnForReturn(CC);
    OutgoingArgHandler Handler(MIRBuilder, MF.getRegInfo(), MIB, AssignFn);
    Success = handleAssignments(MIRBuilder, SplitRetInfos, Handler);
  }

  MIRBuilder.insertInstr(MIB);
  return Success;
}

bool SM83CallLowering::lowerFormalArguments(
    MachineIRBuilder &MIRBuilder, const Function &F,
    ArrayRef<ArrayRef<Register>> VRegs) const {
  auto &DL = F.getParent()->getDataLayout();
  CallingConv::ID CC = F.getCallingConv();
  MachineRegisterInfo &MRI = *MIRBuilder.getMRI();

  SmallVector<ArgInfo, 8> SplitArgs;
  unsigned i = 0;
  for (auto &Arg : F.args()) {
    ArgInfo OrigArg{VRegs[i], Arg.getType()};
    setArgFlags(OrigArg, i + AttributeList::FirstArgIndex, DL, F);

    splitToValueTypes(
      OrigArg, SplitArgs, DL, CC, MRI,
      [&VRegs, &MIRBuilder, i](ArrayRef<Register> Regs, Register DstReg, LLT LLTy, LLT PartLLT,
          int VTSplitIdx) {
        assert(DstReg == VRegs[i][VTSplitIdx]);
        packSplitRegsToOrigType(MIRBuilder, VRegs[i][VTSplitIdx], Regs,
                                LLTy, PartLLT);
      });
    ++i;
  }

  const SM83TargetLowering &TLI = *getTLI<SM83TargetLowering>();
  CCAssignFn *AssignFn = TLI.CCAssignFnForCall(CC, /*IsVarArg=*/false);

  IncomingArgHandler Handler(MIRBuilder, MIRBuilder.getMF().getRegInfo(), AssignFn);
  return handleAssignments(MIRBuilder, SplitArgs, Handler);
}

bool SM83CallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  return false;
}

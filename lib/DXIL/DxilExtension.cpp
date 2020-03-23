///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilExtension.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilExtension.h"

#include "dxc/Support/Global.h"
#include "dxc/dxcapi.internal.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace {

DXIL_SCALAR_TYPE TranslateScalarType(llvm::Type *Ty) {
  DXASSERT_NOMSG(Ty && (Ty->isIntegerTy() || Ty->isFloatingPointTy()));
  DXIL_SCALAR_TYPE DxilTy = {};
  if (Ty->isIntegerTy()) {
    DxilTy.IsFloat = false;
    switch (Ty->getIntegerBitWidth()) {
    case 1:   DxilTy.BitsPow2 = 0;  break;
    case 8:   DxilTy.BitsPow2 = 3;  break;
    case 16:   DxilTy.BitsPow2 = 4;  break;
    case 32:   DxilTy.BitsPow2 = 5;  break;
    case 64:   DxilTy.BitsPow2 = 6;  break;
    default:
      DXASSERT(false, "Unsupported integer bit width.");
    }
  } else if (Ty->isFloatingPointTy()) {
    DxilTy.IsFloat = true;
    if (Ty->isHalfTy())
      DxilTy.BitsPow2 = 4;
    else if (Ty->isFloatTy())
      DxilTy.BitsPow2 = 5;
    else if (Ty->isDoubleTy())
      DxilTy.BitsPow2 = 6;
    else {
      DXASSERT(false, "Unsupported floating-point bit width.");
    }
  }
  return DxilTy;
}

DXIL_SCALAR_CONSTANT TranslateScalarConstant(llvm::Constant *C) {
  DXIL_SCALAR_CONSTANT DxilConst = {};
  DxilConst.Ty = TranslateScalarType(C->getType());
  if (DxilConst.Ty.IsFloat) {
    DxilConst.i64 = cast<ConstantFP>(C)->getValueAPF().bitcastToAPInt().getLimitedValue();
  } else {
    DxilConst.i64 = cast<ConstantInt>(C)->getLimitedValue();
  }
  return DxilConst;
}

llvm::Constant *TranslateScalarConstant(DXIL_SCALAR_CONSTANT DxilConst) {
  if (DxilConst.Ty.IsFloat) {
    // TODO: Bitcast from int
  } else {
    // TODO: Construct int value
  }
  return nullptr;
}

} // anonymous

namespace hlsl {

DxilExtension::DxilExtension(IDxcConstantFoldingExtension *pExt)
  : m_pConstantFoldingExtension(pExt)
{}

Constant *DxilExtension::ConstantFoldScalarCall(llvm::StringRef Name, llvm::Type *Ty, llvm::ArrayRef<llvm::Constant *> RawOperands) {
  DXASSERT_NOMSG(Ty && RawOperands.size() <= UINT32_MAX);
  if (!m_pConstantFoldingExtension)
    return nullptr;
  DXIL_SCALAR_TYPE DxilTy = TranslateScalarType(Ty);
  SmallVector<DXIL_SCALAR_CONSTANT, 16> DxilOperands(RawOperands.size());
  for (auto C : RawOperands) {
    DxilOperands.emplace_back(TranslateScalarConstant(C));
  }
  DXIL_SCALAR_CONSTANT result;
  IFT(m_pConstantFoldingExtension->ConstantFoldScalarCall(
    Name.str().c_str(), DxilTy,
    DxilOperands.data(), (UINT32)DxilOperands.size(),
    &result));
  return TranslateScalarConstant(result);
}
bool DxilExtension::CanConstantFoldCall(const llvm::Function *F) {
  if (!m_pConstantFoldingExtension)
    return false;
  return m_pConstantFoldingExtension->CanConstantFoldCall(F->getName().str().c_str());
}


} // namespace hlsl

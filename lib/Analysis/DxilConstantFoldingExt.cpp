//===-- DxilConstantFoldingExt.cpp - Hooks for extensions ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// These are placeholder hooks to support constant folding of extensions.
// They are defined in a separate file to make it easy to merge changes or link
// in your own version. There should be no upstream changes to these definitions.
//
//===----------------------------------------------------------------------===//
#include "llvm/Analysis/DxilConstantFolding.h"
#include "llvm/IR/Constant.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ArrayRef.h"
#include "dxc/DXIL/DxilExtension.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
using namespace llvm;

Constant *hlsl::ConstantFoldScalarCallExt(StringRef Name, Type *Ty, ArrayRef<Constant *> RawOperands) {
  llvm::LLVMContext &Ctx = Ty->getContext();
  hlsl::DxilExtension *pExt = Ctx.getDxilExtension();
  if (pExt)
    return pExt->ConstantFoldScalarCall(Name, Ty, RawOperands);
  return nullptr;
}

bool hlsl::CanConstantFoldCallToExt(const Function *F) {
  llvm::LLVMContext &Ctx = F->getContext();
  hlsl::DxilExtension *pExt = Ctx.getDxilExtension();
  if (pExt)
    return pExt->CanConstantFoldCall(F);
  return false;
}

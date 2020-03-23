///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilExtension.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Support for dxil extensions.                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ArrayRef.h"

namespace llvm {
class Type;
class Constant;
class Function;
}

struct IDxcConstantFoldingExtension;

namespace hlsl {

// Provide support for private DXIL extensions.
// This currently supports constant folding extended intrinsics.
class DxilExtension {
public:
  DxilExtension(IDxcConstantFoldingExtension*);
  Constant *ConstantFoldScalarCall(llvm::StringRef Name, llvm::Type *Ty, llvm::ArrayRef<llvm::Constant *> RawOperands);
  bool CanConstantFoldCall(const llvm::Function *F);
private:
  IDxcConstantFoldingExtension* m_pConstantFoldingExtension;
};
} // namespace hlsl

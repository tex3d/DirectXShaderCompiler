///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDATDumper.h                                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Use this to dump DxilRuntimeData (RDAT) for testing.                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DumpContext.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/DxilContainer/DxilRuntimeReflection.h"

namespace hlsl {
using namespace RDAT;

namespace dump {

void DumpRuntimeData(const RDAT::DxilRuntimeData &RDAT, DumpContext &d);
template<typename RecordType>
void DumpRecordTable(const RDAT::RDATContext &ctx, DumpContext &d,
                     const char *tableName, const RDAT::TableReader &table);
template<typename RecordType>
void DumpRecordTableEntry(const RDAT::RDATContext &ctx, DumpContext &d,
                          uint32_t i);

template<typename _RecordType>
class RecordDumper : public _RecordType {
public:
  void Dump(const hlsl::RDAT::RDATContext &ctx, DumpContext &d) const;
};

template<typename _RecordType>
class RecordRefDumper : public hlsl::RDAT::RecordRef<_RecordType> {
public:
  RecordRefDumper(uint32_t index) : hlsl::RDAT::RecordRef<_RecordType>(index) {}
  template<typename _DumpTy = _RecordType>
  const char *TypeName(const hlsl::RDAT::RDATContext &ctx) const {
    if (const char *name = TypeNameDerived(ctx))
      return name;
    else if (RecordRef<_DumpTy>(Index).Get(ctx))
      return RecordTraits<_DumpTy>::TypeName();
    return nullptr;
  }
  template<typename _DumpTy = _RecordType>
  void Dump(const hlsl::RDAT::RDATContext &ctx, DumpContext &d) const {
    if (const _DumpTy *ptr = RecordRef<_DumpTy>(Index).Get(ctx)) {
      static_cast< const RecordDumper<_DumpTy>* >(ptr)->Dump(ctx, d);
      DumpDerived(ctx, d);
    }
  }
  // Specialized for base type to recurse into derived
  const char *TypeNameDerived(const hlsl::RDAT::RDATContext &ctx) const { return nullptr; }
  void DumpDerived(const hlsl::RDAT::RDATContext &ctx, DumpContext &d) const {}
};

template<typename _T>
void DumpRecordValue(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                     const char *tyName, const char *memberName,
                     const _T *memberPtr);

template<typename _T>
void DumpRecordRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                   const char *tyName, const char *memberName,
                   hlsl::RDAT::RecordRef<_T> rr);

template<typename _T>
void DumpRecordArrayRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                        const char *tyName, const char *memberName,
                        hlsl::RDAT::RecordArrayRef<_T> rar);

void DumpStringArray(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                     const char *memberName,
                     hlsl::RDAT::RDATStringArray sa);

void DumpBytesRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                   const char *memberName,
                   hlsl::RDAT::BytesRef bytesRef);

template<typename _T>
void DumpValueArray(DumpContext &d, const char *memberName,
                    const char *typeName, const void *valueArray,
                    unsigned arraySize);

#define DEF_RDAT_ENUMS DEF_RDAT_DUMP_DECL
#define DEF_DXIL_ENUMS DEF_RDAT_DUMP_DECL
#include "dxc/DxilContainer/RDAT_Macros.inl"
#include "dxc/DxilContainer/RDAT_LibraryTypes.inl"
#include "dxc/DxilContainer/RDAT_SubobjectTypes.inl"
#undef DEF_RDAT_ENUMS
#undef DEF_DXIL_ENUMS

// Clear macros
#include "dxc/DxilContainer/RDAT_Macros.inl"

} // namespace dump
} // namespace hlsl

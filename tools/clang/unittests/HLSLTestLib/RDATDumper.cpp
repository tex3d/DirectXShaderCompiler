///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDATDumper.cpp                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Use this to dump RDAT data for testing.                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/Global.h"
//#include "assert.h"
#include "dxc/test/RDATDumper.h"

using namespace hlsl;
using namespace RDAT;

namespace hlsl {
namespace dump {

void DumpRuntimeData(const RDAT::DxilRuntimeData &RDAT, DumpContext &d) {
  const RDATContext &ctx = RDAT.GetContext();
  d.WriteLn("DxilRuntimeData:");
  d.Indent();
  DumpRecordTable<RuntimeDataResourceInfo>(ctx, d, "Resources", ctx.Table(RecordTableIndex::ResourceTable));
  DumpRecordTable<RuntimeDataFunctionInfo>(ctx, d, "Functions", ctx.Table(RecordTableIndex::FunctionTable));
  DumpRecordTable<RuntimeDataSubobjectInfo>(ctx, d, "Subobjects", ctx.Table(RecordTableIndex::SubobjectTable));
  d.Dedent();
}

template<typename RecordType>
void DumpRecordTable(const RDAT::RDATContext &ctx, DumpContext &d,
                     const char *tableName, const RDAT::TableReader &table) {
  d.WriteLn("RecordTable ", tableName, "[", table.Count(), "] = {");
  d.Indent();
  for (unsigned i = 0; i < table.Count(); i++) {
    DumpRecordTableEntry<RecordType>(ctx, d, i);
  }
  d.Dedent();
  d.WriteLn("}");
}

template<typename RecordType>
void DumpRecordTableEntry(const RDAT::RDATContext &ctx, DumpContext &d,
                          uint32_t i) {
  // Visit() will prevent recursive/repeated reference expansion.  Resetting
  // for each top-level table entry prevents a record from dumping differently
  // depending on differences in other unrelated records.
  d.VisitReset();

  // RecordRefDumper handles derived types.
  RecordRefDumper<RecordType> rrDumper(i);

  d.WriteLn("<", rrDumper.TypeName(ctx), "-", i, "> = {");
  rrDumper.Dump(ctx, d);
  d.WriteLn("}");
}

template<typename _T>
void DumpRecordValue(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                     const char *tyName, const char *memberName,
                     const _T *memberPtr) {
  d.WriteLn(memberName, ": <", tyName, ">");
  static_cast< const RecordDumper<_T>* >(memberPtr)->Dump(ctx, d);
}

template<typename _T>
void DumpRecordRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                   const char *tyName, const char *memberName,
                   hlsl::RDAT::RecordRef<_T> rr) {
  RecordRefDumper<_T> rrDumper(rr.Index);
  const char *storedTypeName = rrDumper.TypeName(ctx);
  if (nullptr == storedTypeName)
    storedTypeName = tyName;
  // Unique visit location is based on end of struct so derived are not skipped
  if (d.Visit(rr.Get(ctx))) {
    d.WriteLn(memberName, ": <", storedTypeName, "-", rr.Index, "> = {");
    rrDumper.Dump(ctx, d);
    d.WriteLn("}");
  } else {
    d.WriteLn(memberName, ": <", storedTypeName, "-", rr.Index, ">");
  }
}

template<typename _T>
void DumpRecordArrayRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                        const char *tyName, const char *memberName,
                        hlsl::RDAT::RecordArrayRef<_T> rar) {
  auto row = ctx.IndexTable.getRow(rar.Index);
  if (row.Count()) {
    d.WriteLn(memberName, ": <RecordArrayRef<", tyName, ">[", row.Count(), "]-", rar.Index, ">  = {");
    d.Indent();
    for (uint32_t i = 0; i < row.Count(); ++i) {
      RecordRefDumper<_T> rrDumper(row.At(i));
      if (rrDumper.Get(ctx)) {
        if (d.Visit(rrDumper.Get(ctx))) {
          d.WriteLn("[", i, "]: <", rrDumper.TypeName(ctx), "-", rrDumper.Index, "> = {");
          rrDumper.Dump(ctx, d);
          d.WriteLn("}");
        } else {
          d.WriteLn("[", i, "]: <", rrDumper.TypeName(ctx), "-", rrDumper.Index, ">");
        }
      } else {
        d.WriteLn("[", i, "]: <", tyName, "-", row.At(i), "> = <nullptr>");
      }
    }
    d.Dedent();
    d.WriteLn("}");
  } else {
    d.WriteLn(memberName, ": <RecordArrayRef<", tyName, ">[0]> = {}");
  }
}

void DumpStringArray(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                     const char *memberName,
                     hlsl::RDAT::RDATStringArray sa) {
  auto sar = sa.Get(ctx);
  if (sar && sar.Count()) {
    d.WriteLn(memberName, ": <string[", sar.Count(), "]-", sa.Index, "> = {");
    d.Indent();
    for (uint32_t _i = 0; _i < (uint32_t)sar.Count(); ++_i) {
      const char *str = sar[_i];
      if (str) {
        d.WriteLn("[", _i, "]: ", QuotedStringValue(str));
      } else {
        d.WriteLn("[", _i, "]: <nullptr>");
      }
    }
    d.Dedent();
    d.WriteLn("}");
  } else {
    d.WriteLn(memberName, ": <string[0]> = {}");
  }
}

void DumpBytesRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                   const char *memberName,
                   hlsl::RDAT::BytesRef bytesRef) {
  d.WriteLn(memberName, ": <bytes[", bytesRef.Size, "]-", bytesRef.Offset, ">");
}

template<typename _T>
void DumpValueArray(DumpContext &d, const char *memberName,
                    const char *typeName, const void *valueArray,
                    unsigned arraySize) {
  d.WriteLn(memberName": ", typeName, "[", arraySize, "] = { ");
  for (unsigned i = 0; i < arraySize; i++) {
    d.Write(((_T*)valueArray)[i]);
    if (i < arraySize - 1)
      d.Write(", ");
  }
  d.WriteLn(" }")
}

#define DEF_RDAT_TYPES DEF_RDAT_DUMP_IMPL
#define DEF_RDAT_ENUMS DEF_RDAT_DUMP_IMPL
#define DEF_DXIL_ENUMS DEF_RDAT_DUMP_IMPL
#include "dxc/DxilContainer/RDAT_Macros.inl"
#include "dxc/DxilContainer/RDAT_ReflectionTypes.inl"
#include "dxc/DxilContainer/RDAT_LibraryTypes.inl"
#include "dxc/DxilContainer/RDAT_SubobjectTypes.inl"
#undef DEF_RDAT_TYPES
#undef DEF_RDAT_ENUMS
#undef DEF_DXIL_ENUMS

// Clear macros
#include "dxc/DxilContainer/RDAT_Macros.inl"

} // namespace dump
} // namespace hlsl

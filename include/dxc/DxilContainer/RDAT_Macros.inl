///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDAT_Macros.inl                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines macros use to define types Dxil Library Runtime Data (RDAT).      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define GLUE2(a, b) a##b
#define GLUE(a, b) GLUE2(a, b)

#undef RDAT_STRUCT
#undef RDAT_STRUCT_DERIVED
#undef RDAT_STRUCT_TABLE
#undef RDAT_STRUCT_TABLE_DERIVED
#undef RDAT_STRUCT_END
#undef RDAT_UNION
#undef RDAT_UNION_IF
#undef RDAT_UNION_ELIF
#undef RDAT_UNION_ENDIF
#undef RDAT_UNION_END
#undef RDAT_RECORD_REF
#undef RDAT_RECORD_ARRAY_REF
#undef RDAT_RECORD_VALUE
#undef RDAT_STRING
#undef RDAT_STRING_ARRAY_REF
#undef RDAT_VALUE
#undef RDAT_ENUM
#undef RDAT_FLAGS
#undef RDAT_BYTES

#ifdef DEF_RDAT_TYPES

#if DEF_RDAT_TYPES == DEF_RDAT_CLEAR

  #define RDAT_STRUCT(type)
  #define RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_TABLE(type, table) RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table) RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)   // In expr, use pRecord
  #define RDAT_UNION_ELIF(name, expr) // In expr, use pRecord
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)
  #define RDAT_RECORD_ARRAY_REF(type, name)
  #define RDAT_RECORD_VALUE(type, name)
  #define RDAT_STRING(name)
  #define RDAT_STRING_ARRAY_REF(name)
  #define RDAT_VALUE(type, name)
  #define RDAT_ENUM(sTy, eTy, name)
  #define RDAT_FLAGS(sTy, eTy, name)
  #define RDAT_BYTES(name)

#elif DEF_RDAT_TYPES == DEF_RDAT_TYPES_FORWARD_DECL

  #define RDAT_STRUCT(type) struct type;  class type##_Reader;
  #define RDAT_STRUCT_DERIVED(type, base)               RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE(type, table)                RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table)  RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)
  #define RDAT_UNION_ELIF(name, expr)
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)
  #define RDAT_RECORD_ARRAY_REF(type, name)
  #define RDAT_RECORD_VALUE(type, name)
  #define RDAT_STRING(name)
  #define RDAT_STRING_ARRAY_REF(name)
  #define RDAT_VALUE(type, name)
  #define RDAT_ENUM(sTy, eTy, name)
  #define RDAT_FLAGS(sTy, eTy, name)
  #define RDAT_BYTES(name)

#elif DEF_RDAT_TYPES == DEF_RDAT_TYPES_BASIC_STRUCT

  #define RDAT_STRUCT(type)                   struct type {
  #define RDAT_STRUCT_DERIVED(type, base)     \
    struct type : public base {
  #define RDAT_STRUCT_TABLE(type, table) RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table) RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()                   };
  #define RDAT_UNION()                        union {
  #define RDAT_UNION_IF(name, expr)
  #define RDAT_UNION_ELIF(name, expr)
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()                    };
  #define RDAT_RECORD_REF(type, name)         uint32_t name;
  #define RDAT_RECORD_ARRAY_REF(type, name)   uint32_t name;
  #define RDAT_RECORD_VALUE(type, name)       type name;
  #define RDAT_STRING(name)                   uint32_t name;
  #define RDAT_STRING_ARRAY_REF(name)         uint32_t name;
  #define RDAT_VALUE(type, name)              type name;
  #define RDAT_ENUM(sTy, eTy, name)           sTy name;
  #define RDAT_FLAGS(sTy, eTy, name)          sTy name;
  #define RDAT_BYTES(name)                    uint32_t name; uint32_t name##_Size; name;

#elif DEF_RDAT_TYPES == DEF_RDAT_TYPES_USE_HELPERS

  #define RDAT_STRUCT(type)                   struct type {
  #define RDAT_STRUCT_DERIVED(type, base)     struct type : public base {
  #define RDAT_STRUCT_TABLE(type, table)      RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type,     base, table) RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()                   };
  #define RDAT_UNION()                        union {
  #define RDAT_UNION_IF(name, expr)
  #define RDAT_UNION_ELIF(name, expr)
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()                    };
  #define RDAT_RECORD_REF(type, name)         RecordRef<type> name;
  #define RDAT_RECORD_ARRAY_REF(type, name)   RecordArrayRef<type> name;
  #define RDAT_RECORD_VALUE(type, name)       type name;
  #define RDAT_STRING(name)                   RDATString name;
  #define RDAT_STRING_ARRAY_REF(name)         RDATStringArray name;
  #define RDAT_VALUE(type, name)              type name;
  #define RDAT_ENUM(sTy, eTy, name)           sTy name;
  #define RDAT_FLAGS(sTy, eTy, name)          sTy name;
  #define RDAT_BYTES(name)                    hlsl::RDAT::BytesRef name;

#elif DEF_RDAT_TYPES == DEF_RDAT_READER_DECL

  #define RDAT_STRUCT(type)               \
    class type##_Reader : public RecordReader<type##_Reader> { \
    public: \
      typedef type RecordType; \
      type##_Reader(const BaseRecordReader &reader); \
      type##_Reader() : RecordReader<type##_Reader>() {} \
      const RecordType *asRecord() const;
  #define RDAT_STRUCT_DERIVED(type, base) \
    class type##_Reader : public base##Reader { \
    public: \
      typedef type RecordType; \
      typedef base BaseRecordType; \
      type##_Reader(const BaseRecordReader &reader); \
      type##_Reader() : base##Reader() {} \
      const RecordType *asRecord() const;
  #define RDAT_STRUCT_TABLE(type, table)                RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table)  RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END() };
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)           bool has##name() const;
  #define RDAT_UNION_ELIF(name, expr)         RDAT_UNION_IF(name, expr)
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)         type##_Reader get##name() const;
  #define RDAT_RECORD_ARRAY_REF(type, name)   RecordArrayReader<type##_Reader> get##name() const;
  #define RDAT_RECORD_VALUE(type, name)       type##_Reader get##name() const;
  #define RDAT_STRING(name)                   const char *get##name() const;
  #define RDAT_STRING_ARRAY_REF(name)         StringArrayReader get##name() const;
  #define RDAT_VALUE(type, name)              type get##name() const;
  #define RDAT_ENUM(sTy, eTy, name)           eTy get##name() const;
  #define RDAT_FLAGS(sTy, eTy, name)          sTy get##name() const;
  #define RDAT_BYTES(name)                    const void *get##name() const; \
                                              uint32_t size##name() const;

#elif DEF_RDAT_TYPES == DEF_RDAT_READER_IMPL

  #define RDAT_STRUCT(type)               \
      type##_Reader::type##_Reader(const BaseRecordReader &reader) : RecordReader<type##_Reader>(reader) {} \
      const type *type##_Reader::asRecord() const { return BaseRecordReader::asRecord<type>(); }
  #define RDAT_STRUCT_DERIVED(type, base) \
      type##_Reader::type##_Reader(const BaseRecordReader &reader) : RecordReader<type##_Reader>(reader) {} \
      const type *type##_Reader::asRecord() const { return BaseRecordReader::asRecord<type>(); }
  #define RDAT_STRUCT_TABLE(type, table)                RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table)  RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)     bool GLUE(RECORD_TYPE,_Reader)::has##name() const  { if (auto *pRecord = asRecord()) return !!(expr); return false; }
  #define RDAT_UNION_ELIF(name, expr)   RDAT_UNION_IF(name, expr)
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)   type##_Reader GLUE(RECORD_TYPE,_Reader)::get##name() const     { return GetField_RecordRef<type##_Reader>      (&(asRecord()->name)); }
  #define RDAT_RECORD_ARRAY_REF(type, name) \
                        RecordArrayReader<type##_Reader> GLUE(RECORD_TYPE,_Reader)::get##name() const  { return GetField_RecordArrayRef<type##_Reader> (&(asRecord()->name)); }
  #define RDAT_RECORD_VALUE(type, name) type##_Reader GLUE(RECORD_TYPE,_Reader)::get##name() const     { return GetField_RecordValue<type##_Reader>    (&(asRecord()->name)); }
  #define RDAT_STRING(name)             const char *GLUE(RECORD_TYPE,_Reader)::get##name() const       { return GetField_String                        (&(asRecord()->name)); }
  #define RDAT_STRING_ARRAY_REF(name)   StringArrayReader GLUE(RECORD_TYPE,_Reader)::get##name() const { return GetField_StringArray                   (&(asRecord()->name)); }
  #define RDAT_VALUE(type, name)        type GLUE(RECORD_TYPE,_Reader)::get##name() const              { return GetField_Value<type, type>             (&(asRecord()->name)); }
  #define RDAT_ENUM(sTy, eTy, name)     eTy GLUE(RECORD_TYPE,_Reader)::get##name() const               { return GetField_Value<eTy, sTy>               (&(asRecord()->name)); }
  #define RDAT_FLAGS(sTy, eTy, name)    sTy GLUE(RECORD_TYPE,_Reader)::get##name() const               { return GetField_Value<sTy, sTy>               (&(asRecord()->name)); }
  #define RDAT_BYTES(name)              const void *GLUE(RECORD_TYPE,_Reader)::get##name() const       { return GetField_Bytes                         (&(asRecord()->name)); } \
                                        uint32_t GLUE(RECORD_TYPE,_Reader)::size##name() const         { return GetField_BytesSize                     (&(asRecord()->name)); }

#elif DEF_RDAT_TYPES == DEF_RDAT_VALIDATION_DECL

  #define RDAT_STRUCT(type)
  #define RDAT_STRUCT_DERIVED(type, base)                                      \
    template<>                                                                 \
    bool RecursiveRecordValidator::ValidateDerived<base>(const base * pRecord)       \
        const {                                                                \
      return Validate(static_cast<const type*>(pRecord));                             \
    }
  #define RDAT_STRUCT_TABLE(type, table) RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table) RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)
  #define RDAT_UNION_ELIF(name, expr)
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)
  #define RDAT_RECORD_ARRAY_REF(type, name)
  #define RDAT_RECORD_VALUE(type, name)
  #define RDAT_STRING(name)
  #define RDAT_STRING_ARRAY_REF(name)
  #define RDAT_VALUE(type, name)
  #define RDAT_ENUM(sTy, eTy, name)
  #define RDAT_FLAGS(sTy, eTy, name)
  #define RDAT_BYTES(name)

#elif DEF_RDAT_TYPES == DEF_RDAT_VALIDATION_IMPL

  #define RDAT_STRUCT(type) \
    template<> bool ValidateRecord<type>(const RDATContext &ctx, const type *pRecord) {
  #define RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_TABLE(type, table) RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table) RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()                   return true; }
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)           if (expr) {
  #define RDAT_UNION_ELIF(name, expr)         } else if (expr) {
  #define RDAT_UNION_ENDIF()                  }
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)         if (!ValidateRecordRef<type>(ctx, pRecord->name)) return false;
  #define RDAT_RECORD_ARRAY_REF(type, name)   if (!ValidateRecordArrayRef<type>(ctx, pRecord->name)) return false;
  #define RDAT_RECORD_VALUE(type, name)
  #define RDAT_STRING(name)                   if (!ValidateStringRef(ctx, pRecord->name)) return false;
  #define RDAT_STRING_ARRAY_REF(name)         if (!ValidateStringArrayRef(ctx, pRecord->name)) return false;
  #define RDAT_VALUE(type, name)
  #define RDAT_ENUM(sTy, eTy, name)
  #define RDAT_FLAGS(sTy, eTy, name)
  #define RDAT_BYTES(name)

#elif DEF_RDAT_TYPES == DEF_RDAT_DUMP_IMPL

  #define RDAT_STRUCT(type)                   \
    void RecordDumper<hlsl::RDAT::type>::Dump(const hlsl::RDAT::RDATContext &ctx, DumpContext &d) const { \
      d.Indent();  const hlsl::RDAT::type *pRecord = this; (void)pRecord;
  #define RDAT_STRUCT_DERIVED(type, base)     \
    const char *RecordRefDumper<hlsl::RDAT::base>::TypeNameDerived(const hlsl::RDAT::RDATContext &ctx) const { return TypeName<hlsl::RDAT::type>(ctx); } \
    void RecordRefDumper<hlsl::RDAT::base>::DumpDerived(const hlsl::RDAT::RDATContext &ctx, DumpContext &d) const { Dump<hlsl::RDAT::type>(ctx, d); } \
    RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE(type, table) RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table) RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()                   d.Dedent(); }
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)           if (expr) {
  #define RDAT_UNION_ELIF(name, expr)         } else if (expr) {
  #define RDAT_UNION_ENDIF()                  }
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)         DumpRecordRef(ctx, d, #type, #name, name);
  #define RDAT_RECORD_ARRAY_REF(type, name)   DumpRecordArrayRef(ctx, d, #type, #name, name);
  #define RDAT_RECORD_VALUE(type, name)       DumpRecordValue(ctx, d, #type, #name, &name);
  #define RDAT_STRING(name)                   d.WriteLn(#name ": ", QuotedStringValue(name.Get(ctx)));
  #define RDAT_STRING_ARRAY_REF(name)         DumpStringArray(ctx, d, #name, name);
  #define RDAT_VALUE(type, name)              d.WriteLn(#name ": ", name);
  #define RDAT_ENUM(sTy, eTy, name)           d.DumpEnum<eTy>(#name, (eTy)name);
  #define RDAT_FLAGS(sTy, eTy, name)          d.DumpFlags<eTy, sTy>(#name, name);
  #define RDAT_BYTES(name)                    DumpBytesRef(ctx, d, #name, name);

#elif DEF_RDAT_TYPES == DEF_RDAT_TRAITS

  #define RDAT_STRUCT(type) \
    constexpr const char *RecordTraits<type>::TypeName() { return #type; }
  #define RDAT_STRUCT_DERIVED(type, base) RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE(type, table) \
    RDAT_STRUCT(type) \
    constexpr RecordTableIndex RecordTraits<type>::TableIndex() { return RecordTableIndex::table; }
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table) RDAT_STRUCT_TABLE(type, table)
  #define RDAT_STRUCT_END()
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)
  #define RDAT_UNION_ELIF(name, expr)
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)
  #define RDAT_RECORD_ARRAY_REF(type, name)
  #define RDAT_RECORD_VALUE(type, name)
  #define RDAT_STRING(name)
  #define RDAT_STRING_ARRAY_REF(name)
  #define RDAT_VALUE(type, name)
  #define RDAT_ENUM(sTy, eTy, name)
  #define RDAT_FLAGS(sTy, eTy, name)
  #define RDAT_BYTES(name)

#endif // DEF_RDAT_TYPES cases

#endif // DEF_RDAT_TYPES defined

#undef RDAT_ENUM_START
#undef RDAT_DXIL_ENUM_START
#undef RDAT_ENUM_VALUE
#undef RDAT_ENUM_VALUE_ALIAS
#undef RDAT_ENUM_VALUE_NODEF
#undef RDAT_ENUM_END

#ifdef DEF_RDAT_ENUMS

#if DEF_RDAT_ENUMS == DEF_RDAT_CLEAR

  #define RDAT_ENUM_START(eTy, sTy)
  #define RDAT_DXIL_ENUM_START(eTy, sTy)
  #define RDAT_ENUM_VALUE(name, value)        // value only used during declaration
  #define RDAT_ENUM_VALUE_ALIAS(name, value)  // secondary enum names that alias to the same value as another name in the enum
  #define RDAT_ENUM_VALUE_NODEF(name)         // enum names that have no explicitly defined value, or are defined elsewhere
  #define RDAT_ENUM_END()

#elif DEF_RDAT_ENUMS == DEF_RDAT_ENUM_CLASS

  #define RDAT_ENUM_START(eTy, sTy) enum class eTy : sTy {
  #define RDAT_DXIL_ENUM_START(eTy, sTy)      // DXIL enums are defined elsewhere
  #define RDAT_ENUM_VALUE(name, value) name = value,
  #define RDAT_ENUM_VALUE_ALIAS(name, value) name = value,
  #define RDAT_ENUM_VALUE_NODEF(name) name,
  #define RDAT_ENUM_END() };

#elif DEF_RDAT_ENUMS == DEF_RDAT_DUMP_DECL

  #define RDAT_ENUM_START(eTy, sTy) const char *ToString(eTy e);
  #define RDAT_DXIL_ENUM_START(eTy, sTy) const char *ToString(eTy e);
  #define RDAT_ENUM_VALUE(name, value)
  #define RDAT_ENUM_VALUE_ALIAS(name, value)
  #define RDAT_ENUM_VALUE_NODEF(name)
  #define RDAT_ENUM_END()

#elif DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL

  //#define RDAT_ENUM_START(eTy, sTy) \
  //  const char *ToString(eTy e) { \
  //    switch((sTy)e) {
  //#define RDAT_ENUM_VALUE(name, value) case value: return #name;
  #define RDAT_ENUM_START(eTy, sTy) \
    const char *ToString(eTy e) { \
      typedef eTy thisEnumTy; \
      switch(e) {
  #define RDAT_DXIL_ENUM_START(eTy, sTy) \
    const char *ToString(eTy e) { \
      typedef eTy thisEnumTy; \
      switch(e) {
  #define RDAT_ENUM_VALUE_NODEF(name) case thisEnumTy::name: return #name;
  #define RDAT_ENUM_VALUE(name, value) RDAT_ENUM_VALUE_NODEF(name)
  #define RDAT_ENUM_VALUE_ALIAS(name, value)
  #define RDAT_ENUM_END() \
      default: return nullptr; \
      } \
    }

#endif // DEF_RDAT_ENUMS cases

#endif // DEF_RDAT_ENUMS defined


// The idea here is that these can produce several things:
//  - basic C versions of the structures making it easy to see what's really serialized
//  - templated members with Get() methods using context to lookup values in tables
//    - to ease direct use of serialized version with no copying/translation
//  - working version with pointers and such (owned by some context)
//  - serialization functions to translate to/from the working version
//  - code to dump RDAT as text for verification
//  - maybe code to parse RDAT from text?

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
#undef RDAT_VALUE_ARRAY
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
  #define RDAT_VALUE_ARRAY(type, name, asize)
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
  #define RDAT_VALUE_ARRAY(type, name, asize)
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
  #define RDAT_VALUE_ARRAY(type, name, asize) type name[asize];
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
  #define RDAT_VALUE_ARRAY(type, name, asize) type name[asize];
  #define RDAT_ENUM(sTy, eTy, name)           sTy name;
  #define RDAT_FLAGS(sTy, eTy, name)          sTy name;
  #define RDAT_BYTES(name)                    hlsl::RDAT::BytesRef name;

#elif DEF_RDAT_TYPES == DEF_RDAT_READER

// Inline path problematic, attempting separate decl/impl
#if 0
  #define RDAT_STRUCT(type)               \
    class type##_Reader : public RecordReader<type##_Reader> { \
    public: \
      typedef type RecordType; \
      type##_Reader(const BaseRecordReader &reader) : RecordReader<type##_Reader>(reader) {} \
      const RecordType *asRecord() const { return BaseRecordReader::asRecord<RecordType>(); }
  #define RDAT_STRUCT_DERIVED(type, base) \
    class type##_Reader : public base##Reader { \
    public: \
      typedef type RecordType; \
      typedef base BaseRecordType; \
      type##_Reader(const BaseRecordReader &reader) : RecordReader<type##_Reader>(reader) {} \
      const RecordType *asRecord() const { return BaseRecordReader::asRecord<RecordType>(); }
  #define RDAT_STRUCT_TABLE(type, table)                RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table)  RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END() };
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)     bool has##name() const { if (auto *pRecord = asRecord()) return !!(expr); return false; }
  #define RDAT_UNION_ELIF(name, expr)   RDAT_UNION_IF(name, expr)
  #define RDAT_UNION_ENDIF()
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)   type##_Reader get##name() const     { return GetField_RecordRef<type##_Reader>      (&(asRecord()->name)); }
  #define RDAT_RECORD_ARRAY_REF(type, name) \
                        RecordArrayReader<type##_Reader> get##name() const  { return GetField_RecordArrayRef<type##_Reader> (&(asRecord()->name)); }
  #define RDAT_RECORD_VALUE(type, name) type##_Reader get##name() const     { return GetField_RecordValue<type##_Reader>    (&(asRecord()->name)); }
  #define RDAT_STRING(name)             const char *get##name() const       { return GetField_String                        (&(asRecord()->name)); }
  #define RDAT_STRING_ARRAY_REF(name)   StringArrayReader get##name() const { return GetField_StringArray                   (&(asRecord()->name)); }
  #define RDAT_VALUE(type, name)        type get##name() const              { return GetField_Value<type, type>             (&(asRecord()->name)); }
  #define RDAT_VALUE_ARRAY(type, name, asize) \
                                        decltype(auto) get##name() const    { return GetField_ValueArray<type, asize>       (asRecord()->name); }
  #define RDAT_ENUM(sTy, eTy, name)     eTy get##name() const               { return GetField_Value<eTy, sTy>               (&(asRecord()->name)); }
  #define RDAT_FLAGS(sTy, eTy, name)    sTy get##name() const               { return GetField_Value<sTy, sTy>               (&(asRecord()->name)); }
  #define RDAT_BYTES(name)              const void *get##name() const       { return GetField_Bytes                         (&(asRecord()->name)); } \
                                        uint32_t size##name() const         { return GetField_BytesSize                     (&(asRecord()->name)); }
#endif // 0

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
  #define RDAT_VALUE_ARRAY(type, name, asize) const type[asize] &get##name() const;
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
  #define RDAT_VALUE_ARRAY(type, name, asize) \
                                       const type[asize] &GLUE(RECORD_TYPE,_Reader)::get##name() const { return GetField_Value<type, type>             (&(asRecord()->name)); }
  #define RDAT_ENUM(sTy, eTy, name)     eTy GLUE(RECORD_TYPE,_Reader)::get##name() const               { return GetField_Value<eTy, sTy>               (&(asRecord()->name)); }
  #define RDAT_FLAGS(sTy, eTy, name)    sTy GLUE(RECORD_TYPE,_Reader)::get##name() const               { return GetField_Value<sTy, sTy>               (&(asRecord()->name)); }
  #define RDAT_BYTES(name)              const void *GLUE(RECORD_TYPE,_Reader)::get##name() const       { return GetField_Bytes                         (&(asRecord()->name)); } \
                                        uint32_t GLUE(RECORD_TYPE,_Reader)::size##name() const         { return GetField_BytesSize                     (&(asRecord()->name)); }

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
  #define RDAT_VALUE_ARRAY(type, name, asize) DumpValueArray(d, #name, #type, &name);
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

#if 0 // Pointer form incomplete
// TODO: sort out versioning

#elif DEF_RDAT_TYPES == DEF_RDAT_TYPES_USE_POINTERS

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
  #define RDAT_RECORD_REF(type, name)         type *name;
  #define RDAT_RECORD_ARRAY_REF(type, name)   std::vector<type*> name;
  #define RDAT_RECORD_VALUE(type, name)       type name;
  #define RDAT_STRING(name)                   const char *name;
  #define RDAT_STRING_ARRAY_REF(name)         std::vector<const char*> name;
  #define RDAT_VALUE(type, name)              type name;
  #define RDAT_VALUE_ARRAY(type, name, asize) type name[asize];
  #define RDAT_ENUM(sTy, eTy, name)           eTy name;
  #define RDAT_FLAGS(sTy, eTy, name)          sTy name;
  #define RDAT_BYTES(name)                    hlsl::RDAT::BytesPtr name;

#elif DEF_RDAT_TYPES == RDAT_DEF_DESERIALIZE

  // TODO: This could be a method template specialization of ptr_rec::Context instead.
  // #define RDAT_STRUCT(type) template <> bool ptr_rec::Context::Deserialize<ptr_rec::name>( \
  //   const hlp_rec::Context &hlpCtx, const hlp_rec::name &from, ptr_rec::name &to) {

  #define RDAT_STRUCT(type)        \
    bool type::Deserialize( \
      const RDATContext &hlpCtx, ptr_rec::Context &ptrCtx, const hlp_rec::type *pRecord) { \
      if (!this || !pRecord) return false;
  #define RDAT_STRUCT_DERIVED(type, base) RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE(type, table) RDAT_STRUCT(type)
  #define RDAT_STRUCT_TABLE_DERIVED(type, base, table) RDAT_STRUCT_DERIVED(type, base)
  #define RDAT_STRUCT_END()                   return true; }
  #define RDAT_UNION()
  #define RDAT_UNION_IF(name, expr)           if (expr) {
  #define RDAT_UNION_ELIF(name, expr)         } else if (expr) {
  #define RDAT_UNION_ENDIF()                  }
  #define RDAT_UNION_END()
  #define RDAT_RECORD_REF(type, name)         \
      { \
        to->name = ptrCtx.LookupRecord<type>(pRecord->name.Index); \
        if (!ptr_rec::Deserialize_##type(hlpCtx, ptrCtx, pRecord->name.Get(hlpCtx), to->name)) \
          return false; \
      }
  #define RDAT_RECORD_ARRAY_REF(type, name)   \
      { \
        name.clear(); \
        IndexRow _row = hlpCtx.IndexTable.getRow(pRecord->name.Index) \
        name.reserve(row.Count()); \
        for (uint32_t _i = 0; _i < _row.Count(); ++_i) { \
          type *_ptr = ptrCtx.LookupRecord<type>(_row.At(_i)); \
          if (!_ptr_rec::Deserialize_##type(hlpCtx, ptrCtx, pRecord->name.Get(hlpCtx)[_i], _ptr)) \
            return false; \
          name.push_back(_ptr); \
        } \
      }
  #define RDAT_RECORD_VALUE(type, name)   \
      if (!ptr_rec::Deserialize_##type(hlpCtx, ptrCtx, &pRecord->name, &to->name)) \
        return false;
  #define RDAT_STRING(name)                   to->name = ptrCtx.LookupString(pRecord->name;
  #define RDAT_STRING_ARRAY_REF(name)         \
      { \
        name.clear(); \
        IndexRow _row = hlpCtx.IndexTable.getRow(pRecord->name.Index) \
        name.reserve(_row.Count()); \
        for (uint32_t _i = 0; _i < _row.Count(); ++_i) { \
          const char *_ptr = ptrCtx.LookupString(_row.At(_i)); \
          name.push_back(_ptr); \
        } \
      }
  #define RDAT_VALUE(type, name)              to->name = pRecord->name;
  #define RDAT_VALUE_ARRAY(type, name, asize) memcpy(&to->name, &pRecord->name, sizeof(type) * asize);
  #define RDAT_ENUM(sTy, eTy, name)           to->name = (eTy)pRecord->name;
  #define RDAT_FLAGS(sTy, eTy, name)          to->name = pRecord->name;
  #define RDAT_BYTES(name)                    to->name = hlsl::RDAT::BytesPtr(pRecord->name.GetBytes(hlpCtx), to->name.Size);

#endif // 0

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

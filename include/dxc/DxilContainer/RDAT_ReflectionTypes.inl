///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDAT_ReflectionTypes.inl                                                  //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines types used in Dxil Library Runtime Data (RDAT).                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifdef DEF_RDAT_ENUMS

RDAT_ENUM_START(TypeInfoClass, uint32_t)
  RDAT_ENUM_VALUE(Void      , 0)
  RDAT_ENUM_VALUE(Scalar    , 1)  // BasicTypeInfo
  RDAT_ENUM_VALUE(Vector    , 2)  // BasicTypeInfo
  RDAT_ENUM_VALUE(RowMatrix , 3)  // BasicTypeInfo
  RDAT_ENUM_VALUE(ColMatrix , 4)  // BasicTypeInfo
  RDAT_ENUM_VALUE(Struct    , 5)  // StructTypeInfo (StructTypeTable)
  RDAT_ENUM_VALUE(Array     , 6)  // ArrayTypeInfo (ArrayTypeTable)
  RDAT_ENUM_VALUE(Alias     , 7)  // AliasTypeInfo (AliasTypeTable) // TBD: Need this?
//  RDAT_ENUM_VALUE(Object    , 8)  // TBD: Need this?
RDAT_ENUM_END()

#endif // DEF_RDAT_ENUMS

#ifdef DEF_DXIL_ENUMS

RDAT_DXIL_ENUM_START(hlsl::DXIL::ComponentType, uint32_t)
  RDAT_ENUM_VALUE(Invalid, 0)
  RDAT_ENUM_VALUE(I1, 1)
  RDAT_ENUM_VALUE(I16, 2)
  RDAT_ENUM_VALUE(U16, 3)
  RDAT_ENUM_VALUE(I32, 4)
  RDAT_ENUM_VALUE(U32, 5)
  RDAT_ENUM_VALUE(I64, 6)
  RDAT_ENUM_VALUE(U64, 7)
  RDAT_ENUM_VALUE(F16, 8)
  RDAT_ENUM_VALUE(F32, 9)
  RDAT_ENUM_VALUE(F64, 10)
  RDAT_ENUM_VALUE(SNormF16, 11)
  RDAT_ENUM_VALUE(UNormF16, 12)
  RDAT_ENUM_VALUE(SNormF32, 13)
  RDAT_ENUM_VALUE(UNormF32, 14)
  RDAT_ENUM_VALUE(SNormF64, 15)
  RDAT_ENUM_VALUE(UNormF64, 16)
RDAT_ENUM_END()

RDAT_DXIL_ENUM_START(hlsl::DXIL::InterpolationMode, uint8_t)
  RDAT_ENUM_VALUE(Undefined                   , 0)
  RDAT_ENUM_VALUE(Constant                    , 1)
  RDAT_ENUM_VALUE(Linear                      , 2)
  RDAT_ENUM_VALUE(LinearCentroid              , 3)
  RDAT_ENUM_VALUE(LinearNoperspective         , 4)
  RDAT_ENUM_VALUE(LinearNoperspectiveCentroid , 5)
  RDAT_ENUM_VALUE(LinearSample                , 6)
  RDAT_ENUM_VALUE(LinearNoperspectiveSample   , 7)
  RDAT_ENUM_VALUE(Invalid                     , 8)
RDAT_ENUM_END()

#endif // DEF_DXIL_ENUMS

#ifdef DEF_RDAT_TYPES

struct TypeInfo;

#define RECORD_TYPE BasicTypeInfo
RDAT_STRUCT(BasicTypeInfo)
  RDAT_VALUE(uint8_t, Rows)   // matrix rows
  RDAT_VALUE(uint8_t, Cols)   // vector or matrix columns
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, ComponentType)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE ArrayTypeInfo
RDAT_STRUCT_TABLE(ArrayTypeInfo, ArrayTypeTable)
  RDAT_RECORD_REF(TypeInfo, ElementType)
  RDAT_VALUE(uint32_t, ArraySize)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE VarDeclInfo
RDAT_STRUCT_TABLE(VarDeclInfo, VarDeclTable)
  RDAT_STRING(Name)
  RDAT_RECORD_REF(TypeInfo, Type)
  RDAT_VALUE(uint32_t, Offset)            // Offset of member in structure (for legacy cb layout)
  RDAT_VALUE(uint32_t, Flags)             // Member flags, such as CB usage for top-level members (D3D_SHADER_VARIABLE_FLAGS)
  RDAT_ENUM(uint8_t, hlsl::DXIL::InterpolationMode, InterpolationMode)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE StructTypeInfo
RDAT_STRUCT_TABLE(StructTypeInfo, StructTypeTable)
  RDAT_STRING(Name)
  RDAT_RECORD_ARRAY_REF(VarDeclInfo, Members)
RDAT_STRUCT_END()
#undef RECORD_TYPE

// TBD: Do we need AliasType (typedef)?
#define RECORD_TYPE AliasTypeInfo
RDAT_STRUCT_TABLE(AliasTypeInfo, AliasTypeTable)
  RDAT_STRING(Name)
  RDAT_RECORD_REF(TypeInfo, Type)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE TypeInfo
RDAT_STRUCT_TABLE(TypeInfo, TypeTable)
  RDAT_ENUM(uint32_t, hlsl::RDAT::TypeInfoClass, Class)
  RDAT_UNION()
    RDAT_UNION_IF(BasicType, ((uint32_t)pRecord->Class <= (uint32_t)TypeInfoClass::ColMatrix))
      RDAT_RECORD_VALUE(BasicTypeInfo, BasicType)
    RDAT_UNION_ELIF(ArrayType, ((uint32_t)pRecord->Class == (uint32_t)TypeInfoClass::Array))
      RDAT_RECORD_REF(ArrayTypeInfo, ArrayType)
    RDAT_UNION_ELIF(StructType, ((uint32_t)pRecord->Class == (uint32_t)TypeInfoClass::Struct))
      RDAT_RECORD_REF(StructTypeInfo, StructType)
    RDAT_UNION_ENDIF()
  RDAT_UNION_END()
RDAT_STRUCT_END()
#undef RECORD_TYPE

//RDAT_STRUCT_TABLE(ModuleCompilationInfo, ModuleCompilationTable)
//  RDAT_STRING_ARRAY_REF(Arguments)
//RDAT_STRUCT_END()
//
//RDAT_STRUCT_TABLE(ModuleInfo, ModuleTable)
//  RDAT_RECORD_REF(ModuleCompilationInfo, CompilationInfo)
//  RDAT_RECORD_ARRAY_REF(EntryFunctionInfo, EntryFunctions)
//  RDAT_RECORD_REF(ResourceTableInfo, ResourceTable)
//RDAT_STRUCT_END()

#endif // DEF_RDAT_TYPES

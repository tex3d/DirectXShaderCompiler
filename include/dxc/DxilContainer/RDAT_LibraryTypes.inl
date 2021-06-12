///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDAT_LibraryTypes.inl                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines types used in Dxil Library Runtime Data (RDAT).                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifdef DEF_RDAT_ENUMS

RDAT_ENUM_START(DxilResourceFlag, uint32_t)
  RDAT_ENUM_VALUE(None,                     0)
  RDAT_ENUM_VALUE(UAVGloballyCoherent,      1 << 0)
  RDAT_ENUM_VALUE(UAVCounter,               1 << 1)
  RDAT_ENUM_VALUE(UAVRasterizerOrderedView, 1 << 2)
  RDAT_ENUM_VALUE(DynamicIndexing,          1 << 3)
  RDAT_ENUM_VALUE(Atomics64Use,             1 << 4)
RDAT_ENUM_END()

#endif // DEF_RDAT_ENUMS

#ifdef DEF_DXIL_ENUMS

RDAT_DXIL_ENUM_START(hlsl::DXIL::ResourceClass, uint32_t)
  RDAT_ENUM_VALUE_NODEF(SRV)
  RDAT_ENUM_VALUE_NODEF(UAV)
  RDAT_ENUM_VALUE_NODEF(CBuffer)
  RDAT_ENUM_VALUE_NODEF(Sampler)
  RDAT_ENUM_VALUE_NODEF(Invalid)
RDAT_ENUM_END()

RDAT_DXIL_ENUM_START(hlsl::DXIL::ResourceKind, uint32_t)
  RDAT_ENUM_VALUE_NODEF(Invalid)
  RDAT_ENUM_VALUE_NODEF(Texture1D)
  RDAT_ENUM_VALUE_NODEF(Texture2D)
  RDAT_ENUM_VALUE_NODEF(Texture2DMS)
  RDAT_ENUM_VALUE_NODEF(Texture3D)
  RDAT_ENUM_VALUE_NODEF(TextureCube)
  RDAT_ENUM_VALUE_NODEF(Texture1DArray)
  RDAT_ENUM_VALUE_NODEF(Texture2DArray)
  RDAT_ENUM_VALUE_NODEF(Texture2DMSArray)
  RDAT_ENUM_VALUE_NODEF(TextureCubeArray)
  RDAT_ENUM_VALUE_NODEF(TypedBuffer)
  RDAT_ENUM_VALUE_NODEF(RawBuffer)
  RDAT_ENUM_VALUE_NODEF(StructuredBuffer)
  RDAT_ENUM_VALUE_NODEF(CBuffer)
  RDAT_ENUM_VALUE_NODEF(Sampler)
  RDAT_ENUM_VALUE_NODEF(TBuffer)
  RDAT_ENUM_VALUE_NODEF(RTAccelerationStructure)
  RDAT_ENUM_VALUE_NODEF(FeedbackTexture2D)
  RDAT_ENUM_VALUE_NODEF(FeedbackTexture2DArray)
  RDAT_ENUM_VALUE_NODEF(NumEntries)
RDAT_ENUM_END()

RDAT_DXIL_ENUM_START(hlsl::DXIL::ShaderKind, uint32_t)
  RDAT_ENUM_VALUE_NODEF(Pixel)
  RDAT_ENUM_VALUE_NODEF(Vertex)
  RDAT_ENUM_VALUE_NODEF(Geometry)
  RDAT_ENUM_VALUE_NODEF(Hull)
  RDAT_ENUM_VALUE_NODEF(Domain)
  RDAT_ENUM_VALUE_NODEF(Compute)
  RDAT_ENUM_VALUE_NODEF(Library)
  RDAT_ENUM_VALUE_NODEF(RayGeneration)
  RDAT_ENUM_VALUE_NODEF(Intersection)
  RDAT_ENUM_VALUE_NODEF(AnyHit)
  RDAT_ENUM_VALUE_NODEF(ClosestHit)
  RDAT_ENUM_VALUE_NODEF(Miss)
  RDAT_ENUM_VALUE_NODEF(Callable)
  RDAT_ENUM_VALUE_NODEF(Mesh)
  RDAT_ENUM_VALUE_NODEF(Amplification)
  RDAT_ENUM_VALUE_NODEF(Invalid)
RDAT_ENUM_END()

#endif // DEF_DXIL_ENUMS

#ifdef DEF_RDAT_TYPES

#define RECORD_TYPE RuntimeDataResourceInfo
RDAT_STRUCT_TABLE(RuntimeDataResourceInfo, ResourceTable)
  RDAT_ENUM(uint32_t, hlsl::DXIL::ResourceClass, Class)
  RDAT_ENUM(uint32_t, hlsl::DXIL::ResourceKind, Kind)
  RDAT_VALUE(uint32_t, ID)
  RDAT_VALUE(uint32_t, Space)
  RDAT_VALUE(uint32_t, LowerBound)
  RDAT_VALUE(uint32_t, UpperBound)
  RDAT_STRING(Name)
  RDAT_FLAGS(uint32_t, DxilResourceFlag, Flags)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE RuntimeDataFunctionInfo
RDAT_STRUCT_TABLE(RuntimeDataFunctionInfo, FunctionTable)
  // full function name
  RDAT_STRING(Name)
  // unmangled function name
  RDAT_STRING(UnmangledName)
  // list of global resources used by this function
  RDAT_RECORD_ARRAY_REF(RuntimeDataResourceInfo, Resources)
  // list of external function names this function calls
  RDAT_STRING_ARRAY_REF(FunctionDependencies)
  // Shader type, or library function
  RDAT_ENUM(uint32_t, hlsl::DXIL::ShaderKind, ShaderKind)
  // Payload Size:
  // 1) any/closest hit or miss shader: payload size
  // 2) call shader: parameter size 
  RDAT_VALUE(uint32_t, PayloadSizeInBytes)
  // attribute size for closest hit and any hit
  RDAT_VALUE(uint32_t, AttributeSizeInBytes)
  // first 32 bits of feature flag
  RDAT_VALUE(uint32_t, FeatureInfo1)
  // second 32 bits of feature flag
  RDAT_VALUE(uint32_t, FeatureInfo2)
  // valid shader stage flag.
  RDAT_VALUE(uint32_t, ShaderStageFlag)
  // minimum shader target.
  RDAT_VALUE(uint32_t, MinShaderTarget)

#if DEF_RDAT_TYPES == DEF_RDAT_TYPES_USE_HELPERS
  // void SetFeatureFlags(uint64_t flags) convenience method
  void SetFeatureFlags(uint64_t flags) {
    FeatureInfo1 = flags & 0xffffffff;
    FeatureInfo2 = (flags >> 32) & 0xffffffff;
  }
#endif

#if DEF_RDAT_TYPES == DEF_RDAT_READER_DECL
  // uint64_t GetFeatureFlags() convenience method
  uint64_t GetFeatureFlags();
#elif DEF_RDAT_TYPES == DEF_RDAT_READER_IMPL
  // uint64_t GetFeatureFlags() convenience method
  uint64_t RuntimeDataFunctionInfo_Reader::GetFeatureFlags() {
    return asRecord() ? (((uint64_t)asRecord()->FeatureInfo2 << 32) |
                         (uint64_t)asRecord()->FeatureInfo1)
                      : 0;
  }
#endif

RDAT_STRUCT_END()
#undef RECORD_TYPE

#endif // DEF_RDAT_TYPES

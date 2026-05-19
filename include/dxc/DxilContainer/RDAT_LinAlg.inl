///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDAT_LinAlg.inl                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines types used in Dxil Library Runtime Data (RDAT).                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
// Macro indentation makes this file easier to read, but clang-format flattens
// everything.  Turn off clang-format for this file.

#ifdef DEF_RDAT_ENUMS

#ifdef UNIFY_MATRIX_MULTIPLY_STRUCTURES
RDAT_ENUM_START(LinAlgMatrixMultiplyFlag, uint8_t)
  RDAT_ENUM_VALUE(None, 0)
  // The matrix multiply is thread-group-scoped, otherwise it's wave-scoped:
  RDAT_ENUM_VALUE(ThreadGroupScoped, 1 << 0)
RDAT_ENUM_END()
#endif // UNIFY_MATRIX_MULTIPLY_STRUCTURES

RDAT_ENUM_START(LinAlgThreadMatrixVectorMultiplyFlag, uint8_t)
  RDAT_ENUM_VALUE(None, 0)
  // If neither MatrixTransposed or MatrixNonMulOptimalLayout is set, the matrix
  // is loaded from MulOptimal layout.
  // MatrixTransposed: The matrix is loaded from MulOptimalTranspose layout.
  RDAT_ENUM_VALUE(MatrixTransposed, 1 << 0)
  // MatrixNonMulOptimalLayout: The matrix is loaded from a non-MulOptimal
  // layout. Can't be combined with MatrixTransposed flag.
  RDAT_ENUM_VALUE(MatrixNonMulOptimalLayout, 1 << 1)
RDAT_ENUM_END()

RDAT_ENUM_START(LinAlgAccumulateStoreFlag, uint8_t)
  RDAT_ENUM_VALUE(None, 0)
  // MatrixTransposed: Accumulate to OuterProductOptimalTranspose layout,
  // thread-scope only.
  RDAT_ENUM_VALUE(MatrixTransposed, 1 << 0)
  // RawBuffer: Accumulate is to a raw buffer, all scopes.
  RDAT_ENUM_VALUE(RawBuffer, 1 << 1)
  // GroupShared: Accumulate to GroupShared memory, wave/group scope only.
  RDAT_ENUM_VALUE(GroupShared, 1 << 2)
RDAT_ENUM_END()

#endif // DEF_RDAT_ENUMS

#ifdef DEF_DXIL_ENUMS

#endif // DEF_DXIL_ENUMS

#ifdef DEF_RDAT_TYPES

// ------------ LinAlgMatrixOperationShape ------------

#define RECORD_TYPE LinAlgMatrixOperationShape
RDAT_STRUCT_TABLE(LinAlgMatrixOperationShape,
                  LinAlgMatrixOperationShapeTable)
  // For each dimension, Unused == 0
  RDAT_VALUE(uint32_t, M) // Rows in matrix A / Accumulator
  RDAT_VALUE(uint32_t, N) // Columns in matrix B / Accumulator
  RDAT_VALUE(uint32_t, K) // Columns in matrix A / Rows in matrix B
RDAT_STRUCT_END()
#undef RECORD_TYPE

// ------------ LinAlgMatrixConstruction ------------

#define RECORD_TYPE LinAlgMatrixConstruction
RDAT_STRUCT_TABLE(LinAlgMatrixConstruction, LinAlgMatrixConstructionTable)
  // Combined minimum M/N/K across all constructions of the same type.
  // For each, 0 means unused (e.g. K=0 indicates only Accumulator Use)
  RDAT_VALUE(uint32_t, MinM)
  RDAT_VALUE(uint32_t, MinN)
  RDAT_VALUE(uint32_t, MinK)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, MatrixType)
RDAT_STRUCT_END()
#undef RECORD_TYPE

// ------------ LinAlgThreadMatrixVectorMultiply ------------

#define RECORD_TYPE LinAlgThreadMatrixVectorMultiply
RDAT_STRUCT_TABLE(LinAlgThreadMatrixVectorMultiply,
                  LinAlgThreadMatrixVectorMultiplyTable)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, ResultType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, MatrixType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, VectorInputType)
  RDAT_FLAGS(uint8_t, LinAlgThreadMatrixVectorMultiplyFlag, Flags)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#ifdef UNIFY_MATRIX_MULTIPLY_STRUCTURES

// ------------ LinAlgMatrixMultiply ------------

#define RECORD_TYPE LinAlgMatrixMultiply
RDAT_STRUCT_TABLE(LinAlgMatrixMultiply, LinAlgMatrixMultiplyTable)
  RDAT_RECORD_ARRAY_REF(LinAlgMatrixOperationShape, OperationShapes)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, AccumulatorType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, MatrixAType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, MatrixBType)
  RDAT_FLAGS(uint8_t, LinAlgMatrixMultiplyFlag, Flags)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#else

// ------------ LinAlgWaveMatrixMultiply ------------

#define RECORD_TYPE LinAlgWaveMatrixMultiply
RDAT_STRUCT_TABLE(LinAlgWaveMatrixMultiply, LinAlgWaveMatrixMultiplyTable)
  RDAT_RECORD_ARRAY_REF(LinAlgMatrixOperationShape, OperationShapes)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, AccumulatorType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, MatrixAType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, MatrixBType)
RDAT_STRUCT_END()
#undef RECORD_TYPE

// ------------ LinAlgThreadGroupMatrixMultiply ------------

// Since ThreadGroup structure matches Wave structure, these could be shared if
// we want, but would rather do that after eliminating the possibility that they
// could differ for final structures.
#define RECORD_TYPE LinAlgThreadGroupMatrixMultiply
RDAT_STRUCT_TABLE(LinAlgThreadGroupMatrixMultiply,
                  LinAlgThreadGroupMatrixMultiplyTable)
  RDAT_RECORD_ARRAY_REF(LinAlgMatrixOperationShape, OperationShapes)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, AccumulatorType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, MatrixAType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, MatrixBType)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#endif // UNIFY_MATRIX_MULTIPLY_STRUCTURES

// ------------ LinAlgOuterProduct ------------

#define RECORD_TYPE LinAlgOuterProduct
RDAT_STRUCT_TABLE(LinAlgOuterProduct, LinAlgOuterProductTable)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, ResultType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, VectorInputType)
RDAT_STRUCT_END()
#undef RECORD_TYPE

// ------------ LinAlgAccumulateStore ------------

#define RECORD_TYPE LinAlgAccumulateStore
RDAT_STRUCT_TABLE(LinAlgAccumulateStore, LinAlgAccumulateStoreTable)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, AccumulatorType)
  RDAT_FLAGS(uint8_t, LinAlgAccumulateStoreFlag, Flags)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#endif // DEF_RDAT_TYPES

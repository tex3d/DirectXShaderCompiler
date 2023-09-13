// RUN: %dxc -E main -T ps_6_1 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_1 -Od %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E main -T ps_6_1 %s | FileCheck %s -check-prefixes=CHECK,CHK16


// CHK16: Note: shader requires additional functionality:
// CHK16-NEXT: Barycentrics

// a:
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 0, i32 0, i8 0, i8 0)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 0, i32 0, i8 1, i8 0)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 0, i32 0, i8 2, i8 0)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 0, i32 0, i8 3, i8 0)
// b:
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 1, i32 0, i8 0, i8 1)
// ab[1]:
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 2, i32 1, i8 0, i8 2)
// avb[0]:
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 3, i32 0, i8 0, i8 0)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 3, i32 0, i8 1, i8 0)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 3, i32 0, i8 2, i8 0)
// mat:
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 4, i32 0, i8 1, i8 1)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 4, i32 1, i8 1, i8 1)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 4, i32 2, i8 1, i8 1)

struct Input {
  bool b : B;
  bool ab[2] : AB;
  bool3 avb[2] : AVB;
};

float4 main(bool4 a : A, Input b, bool2x3 mat : MAT) : SV_Target
{
  float4 a0 = GetAttributeAtVertex(a, 0);
  float b1 = GetAttributeAtVertex(b.b, 1);
  float bab2 = GetAttributeAtVertex(b.ab[1], 2);
  float3 bavb0 = GetAttributeAtVertex(b.avb[0], 0);
  float3 mat1 = GetAttributeAtVertex(mat[1], 1);

  return a0 + float4(bavb0 + mat1, b1 + bab2);
}

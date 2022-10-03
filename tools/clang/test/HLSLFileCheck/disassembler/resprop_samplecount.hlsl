// RUN: %dxc /T vs_6_6 /E main -DCOUNT= %s | FileCheck %s -DCOUNT=0
// RUN: %dxc /T vs_6_6 /E main -DCOUNT=,0 %s | FileCheck %s -DCOUNT=0
// RUN: %dxc /T vs_6_6 /E main -DCOUNT=,1 %s | FileCheck %s -DCOUNT=1
// RUN: %dxc /T vs_6_6 /E main -DCOUNT=,2 %s | FileCheck %s -DCOUNT=2
// RUN: %dxc /T vs_6_6 /E main -DCOUNT=,7 %s | FileCheck %s -DCOUNT=7
// RUN: %dxc /T vs_6_6 /E main -DCOUNT=,16 %s | FileCheck %s -DCOUNT=16
// RUN: %dxc /T vs_6_6 /E main -DCOUNT=,32 %s | FileCheck %s -DCOUNT=32

// CHECK: call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{[^,]+}}, %dx.types.ResourceProperties { i32 3, i32 {{[0-9]+}} })  ; AnnotateHandle(res,props)  resource: Texture2DMS<4xF32, [[COUNT]]>
Texture2DMS<float4 COUNT> T : register(t0);

float4 main() : OUT
{
  return T.Load(int2(1,2), 0);
}

// RUN: %dxc -T lib_6_3 -DDEF_FN1 -Fo lib1 %s         | FileCheck %s -check-prefix=CHECKNOBIND
// RUN: %dxc -T lib_6_3 -DBIND -DDEF_MAIN -Fo lib2 %s | FileCheck %s -check-prefix=CHECKBIND
// RUN: %dxl -E main -T ps_6_0 lib1;lib2 %s           | FileCheck %s -check-prefixes=CHECKBIND,CHECK

// lib1 with fn1 has unallocated resources
// lib2 with fn2 and main that calls fn1 has bindings on resources, and fn2 is not called.
// Make sure final shader has bindings from lib2

// RUN: %dxc -T lib_6_3 -DBIND -DDEF_FN1 -Fo lib1 %s                  | FileCheck %s -check-prefix=CHECKBIND
// RUN: %dxc -T lib_6_3 -auto-binding-space 13 -DDEF_MAIN -Fo lib2 %s | FileCheck %s -check-prefix=CHECKAUTOBIND
// RUN: %dxl -E main -T ps_6_0 lib1;lib2 %s                           | FileCheck %s -check-prefixes=CHECKBIND,CHECK

// lib1 compiled with explicit bindings
// lib2 uses -auto-binding-space 13, but resources are not used from lib2
// Make sure final shader has bindings from lib1, and conflicting bindings
// on unused resources don't create a conflict

// Common Checks:
// CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,

// Check various binding scenarios
// CHECKNOBIND-DAG: !{i32 0, %"class.Texture2D<vector<float, 4> >"* {{.*}}, !"g_txDiffuse", i32 -1, i32 -1, i32 1,
// CHECKNOBIND-DAG: !{i32 0, %struct.SamplerState* {{.*}}, !"g_samLinear", i32 -1, i32 -1, i32 1,
// CHECKBIND-DAG: !{i32 0, %"class.Texture2D<vector<float, 4> >"* {{.*}}, !"g_txDiffuse", i32 0, i32 23, i32 1,
// CHECKBIND-DAG: !{i32 0, %struct.SamplerState* {{.*}}, !"g_samLinear", i32 0, i32 31, i32 1,
// CHECKAUTOBIND-DAG: !{i32 0, %"class.Texture2D<vector<float, 4> >"* {{.*}}, !"g_txDiffuse", i32 13, i32 0, i32 1,
// CHECKAUTOBIND-DAG: !{i32 0, %struct.SamplerState* {{.*}}, !"g_samLinear", i32 13, i32 0, i32 1,


#ifdef BIND
#define BINDT : register(t23)
#define BINDS : register(s31)
#else
#define BINDT
#define BINDS
#endif

Texture2D    g_txDiffuse BINDT;
SamplerState    g_samLinear BINDS;

#ifdef DEF_FN1
export float4 fn1(float2 coord) {
  return g_txDiffuse.Sample(g_samLinear, coord);
}
#else
float4 fn1(float2 coord);

export float4 fn2(float2 coord) {
  return g_txDiffuse.Sample(g_samLinear, coord);
}
#endif // DEF_FN1

#ifdef DEF_MAIN
[shader("pixel")]
float4 main(float2 c : C) : SV_TARGET
{
  return fn1(c);
}
#endif // DEF_MAIN

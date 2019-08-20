// RUN: %dxc -E amplification -T as_6_5 %s | FileCheck %s

// CHECK: define void @amplification

struct MeshPayload
{
  float4 data;
  float3x4 mat;
};

groupshared MeshPayload pld;

[numthreads(4,3,1)]
void amplification(uint3 gtid : SV_GroupThreadID)
{
  pld.data[gtid.x] = gtid.x;
  pld.mat[gtid.y][gtid.x] = gtid.x * gtid.y + gtid.x;
  DispatchMesh(1,1,1,pld);
}

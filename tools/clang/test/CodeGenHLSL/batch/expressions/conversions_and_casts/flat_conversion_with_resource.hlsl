// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

Texture2D g_T;
struct Base { Texture2D T; };
struct Derived : Base {};

Derived New() {
  Base b;   // Base type
  b.T = g_T;
  return b; // FlatConversion to Derived, containing resource
}

float4 main(uint3 coord : COORD) : SV_Target {
  Derived d = New();
  return d.T.Load(coord);
}

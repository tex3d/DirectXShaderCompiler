// RUN: %dxc -Od -Zi -Qembed_debug -E main -T ps_6_0 %s | FileCheck %s

// Make sure the bit pieces have the offset in bits
// CHECK-DAG: !{{[0-9]+}} = !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: !{{[0-9]+}} = !DIExpression(DW_OP_bit_piece, 32, 32)

struct S {
  float foo;
  float bar;
};

float main(float a : A) : SV_Target {
  S s;
  s.foo = a * a;
  s.bar = a + a;
  return s.foo + s.bar;
}


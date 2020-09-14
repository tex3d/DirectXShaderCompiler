
#ifdef BINDING_MISMATCH
#define BINDING b1
#else
#define BINDING b0
#endif

#ifdef STRUCT_NAME_MISMATCH
#define STRUCT_NAME S1
#else
#define STRUCT_NAME S
#endif

struct STRUCT_NAME
{
#ifdef TYPE_MISMATCH
    int i;
#endif
    float values;
};

ConstantBuffer<STRUCT_NAME> g_buf : register(BINDING);
RWByteAddressBuffer b : register(u0);

[shader("compute")]
[numthreads(32, 1, 1)]
void main2(uint dtid : SV_DispatchThreadId)
{
    b.Store(dtid * 4, g_buf.values);
}

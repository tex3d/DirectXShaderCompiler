
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

export
float Extern(uint dtid)
{
     return g_buf.values + dtid;
}

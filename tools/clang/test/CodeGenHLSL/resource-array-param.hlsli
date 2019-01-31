// Defines:
// GDIM/LDIM/PDIM = global/local/parameter variable texture array dimension (4|)
// LINIT = initializer for local variable (gTex|{gTex[0],gTex[1],gTex[2],gTex[3]})
// PMOD = parameter modifier (in|inout)
// MODIDX = [optional] modify texture before load with pTex[0]=pTex[MODIDX] first
// ARG = argument to pass to lookup (gTex|lTex)

#ifndef LDIM
#define LDIM GDIM
#endif

#ifndef LINIT
#define LINIT gTex
#endif

#ifndef PMOD
#define PMOD in
#endif

Texture2D gTex[GDIM];

float4 lookup(PMOD Texture2D pTex[PDIM], int3 coord) {
#ifdef MODIDX
  pTex[0] = pTex[MODIDX];
#endif // MODIDX
  return pTex[0].Load(coord);
}

float4 main() : SV_Target
{
  float4 result = 0;
  Texture2D lTex[LDIM] = LINIT;
  result += lookup(ARG, 1);
  return result;
}

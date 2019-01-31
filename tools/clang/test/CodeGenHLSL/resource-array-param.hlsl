// RUN: %dxc -E main -T ps_6_0 %s

// switches:
// As = assign
// Glob = global texture array
// In = in texture array parameter
// Io = inout texture array parameter
// Loc = local texture array parameter
// Unb = unbounded
// 4 = array size 4
// GlobIo = pass global to inout argument

// fxc 5.0 pass:
// /T ps_5_0 /DLoc /DAsLoc /DAsIo4 /DLocUnb

// fxc 5.1 pass:
// /T ps_5_1 /DInUnb /DIoUnb /DLoc /DLocUnb
// /T ps_5_1 /enable_unbounded_descriptor_tables /DGlobUnb /DInUnb /DIoUnb /DLoc /DLocUnb

// fxc 5.0 failures
// GlobIo: error X3025: out parameters require l-value arguments (given argument is implicitly const, such as a global)
// InUnb: error X3072: 'tex': array dimensions of function parameters must be explicit
// IoUnb: error X3072: 'tex': array dimensions of function parameters must be explicit

// fxc 5.1 failures
// GlobIo: error X3025: out parameters require l-value arguments (given argument is implicitly const, such as a global)
// As*: error X3500: array reference cannot be used as an l-value; not natively addressable

Texture2D Tex4[4];
#ifdef GlobUnb
// fxc 5.0: error X3074: 'Tex': implicit array missing initial value
Texture2D Tex[];
#endif // GlobUnb

#ifndef InUnb
#ifdef AsInUnb
#define InUnb
#endif
#endif // InUnb

#ifndef Loc
#ifdef AsLoc
#define Loc
#endif
#endif // Loc

#ifdef InUnb
// fxc 5.0: error X3072: 'tex': array dimensions of function parameters must be explicit
float4 lookup(Texture2D tex[], int3 coord) {
  #ifdef AsInUnb
  // fxc 5.1: error X3500: array reference cannot be used as an l-value; not natively addressable
    tex[0] = tex[1];
  #endif // AsInUnb
  return tex[0].Load(coord);
}
#endif // InUnb

float4 lookup4(Texture2D tex[4], int3 coord) {
#ifdef AsIn4
  // fxc 5.0: pass
  // fxc 5.1: error X3500: array reference cannot be used as an l-value; not natively addressable
  tex[0] = tex[1];
#endif // AsIn4
  return tex[0].Load(coord);
}

// Version with texture array as inout param, but only read from it
#ifdef IoUnb
// fxc 5.0: error X3072: 'tex': array dimensions of function parameters must be explicit
// fxc 5.1: pass
float4 lookup_io(inout Texture2D tex[], int3 coord) {
  float4 result = tex[0].Load(coord);
  return result;
}
#endif // IoUnb

float4 lookup4_io(inout Texture2D tex[4], int3 coord) {
  float4 result = tex[0].Load(coord);
  return result;
}

// These modify the inout texture array
#ifdef AsIoUnb
// fxc 5.0: error X3072: 'tex': array dimensions of function parameters must be explicit
// fxc 5.1: error X3500: array reference cannot be used as an l-value; not natively addressable
float4 lookup_iom(inout Texture2D tex[], int3 coord, int nextIdx) {
  float4 result = tex[0].Load(coord);
  tex[0] = tex[nextIdx];
  return result;
}
#endif // AsIoUnb

#ifdef AsIo4
// fxc 5.1: error X3500: array reference cannot be used as an l-value; not natively addressable
float4 lookup4_iom(inout Texture2D tex[4], int3 coord, int nextIdx) {
  float4 result = tex[0].Load(coord);
  tex[0] = tex[nextIdx];
  return result;
}
#endif // AsIo4

float4 main() : SV_Target
{
  float4 result = 0;
  result += lookup4(Tex4, 11);
#ifdef GlobUnb
  result += lookup4(Tex, 12);
#endif // GlobUnb
#ifdef InUnb
  result += lookup(Tex4, 13);
  #ifdef GlobUnb
    result += lookup(Tex, 14);
  #endif // GlobUnb
#endif // InUnb

  // Try inout resource array with global
#ifdef GlobIo
  // fxc 5.x: error X3025: out parameters require l-value arguments (given argument is implicitly const, such as a global)
  result += lookup4_io(Tex4, 21);
  #ifdef GlobUnb
    result += lookup4_io(Tex, 22);
  #endif // GlobUnb
  #ifdef IoUnb
    result += lookup_io(Tex4, 23);
    #ifdef GlobUnb
      result += lookup_io(Tex, 24);
    #endif // GlobUnb
  #endif // IoUnb
#endif // GlobIo

  // Try inout resource array with local
#ifdef Loc
  // fxc 5.x: pass
  Texture2D localTex4[4] = Tex4;

  #ifdef IoUnb
    // fxc 5.0: error X3072: 'tex': array dimensions of function parameters must be explicit
    // fxc 5.1: pass
    result += lookup_io(localTex4, 31);
  #endif // IoUnb

  #ifdef AsLoc
    // fxc 5.0: pass
    // fxc 5.1: error X3500: array reference cannot be used as an l-value; not natively addressable
    localTex4[0] = Tex4[0];
    result += lookup4_io(localTex4, 32);
  #endif // AsLoc

  #ifdef AsIo4
    // fxc 5.0: pass
    // fxc 5.1: error X3500: array reference cannot be used as an l-value; not natively addressable
    result += lookup4_iom(localTex4, 33, 1);
  #endif // AsIo4
  #ifdef AsIoUnb
    result += lookup_iom(localTex4, 34, 2);
  #endif // AsIoUnb

  result += lookup4_io(localTex4, 35);
#endif // Loc

  // localTex[] is like a reference to the global array, and cannot be modified.
#ifdef LocUnb
  Texture2D localTex[] =
  #ifdef GlobUnb
      // fxc 5.1: pass
      Tex
  #else // GlobUnb
      // fxc 5.0: pass
      // fxc 5.1: pass
      Tex4
  #endif // GlobUnb
    ;
  #ifdef IoUnb
    result += lookup_io(localTex, 41);
  #endif // IoUnb
  #ifdef AsIo4
    result += lookup4_iom(localTex, 42, 3);
  #endif // AsIo4
  #ifdef AsIoUnb
    result += lookup_iom(localTex, 43, 2);
  #endif // AsIoUnb

  result += lookup4(localTex, 44);
  result += lookup4_io(localTex, 45);
#endif // LocUnb

  return result;
}

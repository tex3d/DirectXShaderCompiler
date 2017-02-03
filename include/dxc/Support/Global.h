///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Global.h                                                                  //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides important declarations global to all DX Compiler code.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Redeclare some macros to not depend on winerror.h
#define DXC_FAILED(hr) (((HRESULT)(hr)) < 0)
#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
#ifndef _Return_type_success_
typedef long HRESULT;
#else
typedef _Return_type_success_(return >= 0) long HRESULT;
#endif // _Return_type_success_
#endif // !_HRESULT_DEFINED

#include <stdarg.h>
#include "dxc/Support/Exception.h"

namespace std { class error_code; }
void CheckLLVMErrorCode(const std::error_code &ec);


/******************************************************************************
                          Project-wide macros
******************************************************************************/
#define SAFE_RELEASE(p)       { if (p) { (p)->Release();  (p) = nullptr; } }
#define SAFE_ADDREF(p)        { if (p) { (p)->AddRef();               } }

#define SAFE_DELETE_ARRAY(p)  { delete [](p); p = nullptr; }
#define SAFE_DELETE(p)        { delete (p); p = nullptr;  }

// VH is used in other DXC projects, but it's also a typedef in llvm.
// Use the IFC (IfFailedCleanup) set of conventions.
#define IFC(x)      { hr = (x); if (DXC_FAILED(hr)) goto Cleanup; }
#define IFR(x)      { HRESULT __hr = (x); if (DXC_FAILED(__hr)) return __hr; }
#define IFRBOOL(x,y){ if (!(x)) return (y); }
#define IFCBOOL(x,y){ if (!(x)) { hr = (y); goto Cleanup; } }
#define IFCOOM(x)   { if (nullptr == (x)) { hr = E_OUTOFMEMORY; goto Cleanup; } }
#define IFROOM(x)   { if (nullptr == (x)) { return E_OUTOFMEMORY; } }
#define IFCPTR(x)   { if (nullptr == (x)) { hr = E_POINTER; goto Cleanup; }}
#define IFT(x)      { HRESULT __hr = (x); if (DXC_FAILED(__hr)) throw ::hlsl::Exception(__hr); }
#define IFTBOOL(x,y){ if (!(x)) throw ::hlsl::Exception(y); }
#define IFTOOM(x)   { if (nullptr == (x)) { throw ::hlsl::Exception(E_OUTOFMEMORY); }}
#define IFTPTR(x)   { if (nullptr == (x)) { throw ::hlsl::Exception(E_POINTER); }}
#define IFTARG(x)   { if (!(x)) { throw ::hlsl::Exception(E_INVALIDARG); }}
#define IFTLLVM(x)  { CheckLLVMErrorCode(x); }
#define IFTMSG(x, msg) { HRESULT __hr = (x); if (DXC_FAILED(__hr)) throw ::hlsl::Exception(__hr, msg); }
#define IFTBOOLMSG(x, y, msg) { if (!(x)) throw ::hlsl::Exception(y, msg); }

// Propagate an C++ exception into an HRESULT.
#define CATCH_CPP_ASSIGN_HRESULT() \
  catch (std::bad_alloc&)                   { hr = E_OUTOFMEMORY; } \
  catch (hlsl::Exception& _hlsl_exception_) {                       \
    _Analysis_assume_(DXC_FAILED(_hlsl_exception_.hr));             \
    return hr = _hlsl_exception_.hr;                                \
  }                                                                 \
  catch (...)                               { hr = E_FAIL; }
#define CATCH_CPP_RETURN_HRESULT() \
  catch (std::bad_alloc&)                   { return E_OUTOFMEMORY; } \
  catch (hlsl::Exception& _hlsl_exception_) {                         \
    _Analysis_assume_(DXC_FAILED(_hlsl_exception_.hr));               \
    return _hlsl_exception_.hr;                                       \
  }                                                                   \
  catch (...)                               { return E_FAIL; }

template<typename T> T *VerifyNullAndThrow(T *p) {
  if (p == nullptr)
    throw std::bad_alloc();
  return p;
}
#define VNT(__p) VerifyNullAndThrow(__p)

extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(_In_opt_ const char *msg);

inline void OutputDebugBytes(const void *ptr, size_t len) {
  const char digits[] = "0123456789abcdef";
  const unsigned char *pBytes = (const unsigned char *)ptr;
  const int bytesPerLine = 16;
  char buffer[bytesPerLine * 3 + 2 + 1];
  buffer[_countof(buffer) - 3] = '\r';
  buffer[_countof(buffer) - 2] = '\n';
  buffer[_countof(buffer) - 1] = '\0';

  char *pWrite = buffer;
  char *pEnd = buffer + _countof(buffer) - 3;
  while (len) {
    *pWrite++ = digits[(*pBytes & 0xF0) >> 4];
    *pWrite++ = digits[*pBytes & 0x0f];
    *pWrite++ = ' ';
    if (pWrite == pEnd) {
      OutputDebugStringA(buffer);
      pWrite = buffer;
    }
    --len;
    ++pBytes;
  }
  if (pWrite != buffer) {
    *pWrite = '\0';
    OutputDebugStringA(buffer);
    OutputDebugStringA("\r\n");
  }
}

inline void OutputDebugFormatA(_In_ _Printf_format_string_ _Null_terminated_ const char* pszFormat, ...) {
  char buffer[1024];

  va_list argList;
  va_start(argList, pszFormat);
  int count = vsprintf_s(buffer, _countof(buffer), pszFormat, argList);
  va_end(argList);

  OutputDebugStringA(buffer);
  if (count < 0) {
    OutputDebugStringA("...\n");
  }
}

#ifdef DBG

// DXASSERT is used to debug break when 'exp' evaluates to false and is only
//     intended for internal developer use. It is compiled out in free
//     builds.  This means that code that should be in the final exe should
//     NOT be inside of of an DXASSERT test.
//
// 'fmt' is a printf-like format string; positional arguments aren't
//  supported.
//
// Example: DXASSERT(i > 10, "Hello %s", "World");
// This prints 'Hello World (i > 10)' and breaks in the debugger if the
// assertion doesn't hold.
//
#define DXASSERT(exp, fmt, ...)\
  do { _Analysis_assume_(exp); if(!(exp)) {                              \
    OutputDebugFormatA("Error: \t%s\t\nFile:\n%s(%d)\t\nFunc:\t%s.\n\t" fmt "\n", "!(" #exp ")", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    __debugbreak();\
  } } while(0)

#define DXASSERT_LOCALVAR(local, exp, s, ...) DXASSERT(exp, s, __VA_ARGS__)

#define DXASSERT_NOMSG(exp) DXASSERT(exp, "")

#define DXVERIFY_NOMSG(exp) DXASSERT(exp, "")

// This should be improved with global enabled mask rather than a compile-time mask.
#define DXTRACE_MASK_ENABLED  0
#define DXTRACE_MASK_APIFS    1
#define DXTRACE_ENABLED(subsystem) (DXTRACE_MASK_ENABLED & subsystem)

// DXTRACE_FMT formats a debugger trace message if DXTRACE_MASK allows it.
#define DXTRACE_FMT(subsystem, fmt, ...) do { \
  if (DXTRACE_ENABLED(subsystem)) OutputDebugFormatA(fmt, __VA_ARGS__); \
} while (0)
/// DXTRACE_FMT_APIFS is used by the API-based virtual filesystem.
#define DXTRACE_FMT_APIFS(fmt, ...) DXTRACE_FMT(DXTRACE_MASK_APIFS, fmt, __VA_ARGS__)

#else

// DXASSERT is disabled in free builds.
#define DXASSERT(exp, s, ...) _Analysis_assume_(exp)

// DXASSERT_LOCALVAR is disabled in free builds, but we keep the local referenced to avoid a warning.
#define DXASSERT_LOCALVAR(local, exp, s, ...) do { (local); _Analysis_assume_(exp); } while (0)

// DXASSERT_NOMSG is disabled in free builds.
#define DXASSERT_NOMSG(exp) _Analysis_assume_(exp)

// DXVERIFY is patterned after NT_VERIFY and will evaluate the expression
#define DXVERIFY_NOMSG(exp) do { (exp); _Analysis_assume_(exp); } while (0)

// DXTRACE_FMT and the subsystem versions are disabled in free builds.
#define DXTRACE_FMT(...) (void)(0)
#define DXTRACE_FMT_APIFS(...) (void)(0)

#endif // DBG

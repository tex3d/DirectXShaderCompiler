///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcapi.extval.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Extends support for DXC API for overriding the validator.                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/dxcapi.extval.h"

namespace dxc {

DxcDllExtValidationSupport::DxcDllExtValidationSupport() : DxcDllSupport() {}

DxcDllExtValidationSupport::~DxcDllExtValidationSupport() { Cleanup(); }

HRESULT DxcDllExtValidationSupport::InitializeInternal(LPCSTR DllName,
                                                       LPCSTR FnName) {
  HRESULT HR = DxcDllSupport::InitializeInternal(DllName, FnName);

  if (FAILED(HR))
    return HR;

  // Get path from environment variable, if set.  If unset, no external
  // validator will be used.
  char *envPath = getenv("DXC_DXIL_DLL_PATH");
  if (envPath != nullptr)
    ExternalValidatorPath = envPath;
  else
    return S_OK;

  // Load the external validator DLL
  HR = DxilSupport.InitializeForDll(ExternalValidatorPath.c_str(),
                                    "DxcCreateInstance");

  // The caller should check FailedInitForExtValidation for failure,
  // but this interface will still work using internal validation.
  // S_FALSE is a passing return code that's different from S_OK.
  if (FAILED(HR))
    return S_FALSE;

  return S_OK;
}

HRESULT DxcDllExtValidationSupport::CreateInstance(REFCLSID clsid, REFIID riid,
                                                   IUnknown **pResult) {
  return DxcDllSupport::CreateInstance(clsid, riid, pResult);
}

HRESULT DxcDllExtValidationSupport::CreateInstance2(IMalloc *pMalloc,
                                                    REFCLSID clsid, REFIID riid,
                                                    IUnknown **pResult) {
  return DxcDllSupport::CreateInstance2(pMalloc, clsid, riid, pResult);
}

void DxcDllExtValidationSupport::Cleanup() {
  DxcDllSupport::Cleanup();
  DxilSupport.Cleanup();
}

HMODULE DxcDllExtValidationSupport::Detach() {
  HMODULE baseHandle = DxcDllSupport::Detach();

  // Cleanup the external validator DLL if it was loaded.
  DxilSupport.Cleanup();

  return baseHandle;
}

bool DxcDllExtValidationSupport::FailedInitForExtValidation() const {
  return !ExternalValidatorPath.empty() && !DxilSupport.IsEnabled();
}

bool IsWrappedForExternalValidation(IUnknown *pInterface) {
  // Stub implementation
  // Check if the interface is wrapped for external validation
  return false;
}

} // namespace dxc
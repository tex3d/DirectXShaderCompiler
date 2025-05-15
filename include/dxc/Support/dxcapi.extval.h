//////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcapi.extval.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Extends support for DXC API for overriding the validator.                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __DXCAPI_EXTVAL_H__
#define __DXCAPI_EXTVAL_H__

#include "dxc/Support/dxcapi.use.h"
#include <string>

namespace dxc {

class DxcDllExtValidationSupport : public DxcDllSupport {
protected:
  std::string ExternalValidatorPath;
  DxcDllSupport DxilSupport;

  HRESULT InitializeInternal(LPCSTR dllName, LPCSTR fnName) override;

public:
  DxcDllExtValidationSupport() : DxcDllSupport() {}
  ~DxcDllExtValidationSupport() override;
  
  HRESULT CreateInstance(REFCLSID clsid, REFIID riid,
                         IUnknown **pResult) override;
  HRESULT CreateInstance2(IMalloc *pMalloc, REFCLSID clsid, REFIID riid,
                          IUnknown **pResult) override;
  void Cleanup() override;
  HMODULE Detach() override;

  bool FailedInitForExtValidation() const;
};

/// This function is used to determine if the interface is wrapped for external
/// validation.  This can be used by testing, or other code that needs to
/// determine if the interface is wrapped for external validation.
/// \param pInterface The interface to check.
/// \returns true if the interface is wrapped for external validation.
/// \returns false if the interface is not wrapped for external validation.
bool IsWrappedForExternalValidation(IUnknown *pInterface);

} // namespace dxc

#endif

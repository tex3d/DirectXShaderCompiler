///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcvalidator.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DirectX Validator object.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DiagnosticPrinter.h"

#include "dxc/Support/WinIncludes.h"
#include "dxc/HLSL/DxilContainer.h"
#include "dxc/HLSL/DxilValidation.h"

#include "dxc/Support/Global.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"
#include "dxc/Support/microcom.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/dxcapi.impl.h"
#include "dxc/HLSL/DxilRootSignature.h"
#include "dxcetw.h"

using namespace llvm;
using namespace hlsl;

class PrintDiagnosticContext {
private:
  DiagnosticPrinter &m_Printer;
  bool m_errorsFound;
  bool m_warningsFound;
public:
  PrintDiagnosticContext(DiagnosticPrinter &printer)
      : m_Printer(printer), m_errorsFound(false), m_warningsFound(false) {}

  bool HasErrors() const {
    return m_errorsFound;
  }
  bool HasWarnings() const {
    return m_warningsFound;
  }
  void Handle(const DiagnosticInfo &DI) {
    DI.print(m_Printer);
    switch (DI.getSeverity()) {
    case llvm::DiagnosticSeverity::DS_Error:
      m_errorsFound = true;
      break;
    case llvm::DiagnosticSeverity::DS_Warning:
      m_warningsFound = true;
      break;
    }
    m_Printer << "\n";
  }
};

static void PrintDiagnosticHandler(const DiagnosticInfo &DI, void *Context) {
  reinterpret_cast<PrintDiagnosticContext *>(Context)->Handle(DI);
}

// Utility class for setting and restoring the diagnostic context so we may capture errors/warnings
struct DiagRestore {
  LLVMContext &Ctx;
  void *OrigDiagContext;
  LLVMContext::DiagnosticHandlerTy OrigHandler;

  DiagRestore(llvm::LLVMContext &Ctx, void *DiagContext) : Ctx(Ctx) {
    OrigHandler = Ctx.getDiagnosticHandler();
    OrigDiagContext = Ctx.getDiagnosticContext();
    Ctx.setDiagnosticHandler(PrintDiagnosticHandler, DiagContext);
  }
  ~DiagRestore() {
    Ctx.setDiagnosticHandler(OrigHandler, OrigDiagContext);
  }
};

class DxcValidator : public IDxcValidator, public IDxcVersionInfo {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)

  HRESULT RunValidation(
    _In_ IDxcBlob *pShader,                       // Shader to validate.
    _In_ llvm::Module *pModule,                   // Module to validate, if available.
    _In_ llvm::Module *pDebugModule,              // Debug module to validate, if available
    _In_ AbstractMemoryStream *pDiagStream);

  HRESULT RunRootSignatureValidation(
    _In_ IDxcBlob *pShader,                       // Shader to validate.
    _In_ AbstractMemoryStream *pDiagStream);

public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  DxcValidator() : m_dwRef(0) {}

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject) {
    return DoBasicQueryInterface2<IDxcValidator, IDxcVersionInfo>(this, iid, ppvObject);
  }

  // For internal use only.
  HRESULT ValidateWithOptModules(
    _In_ IDxcBlob *pShader,                       // Shader to validate.
    _In_ UINT32 Flags,                            // Validation flags.
    _In_ llvm::Module *pModule,                   // Module to validate, if available.
    _In_ llvm::Module *pDebugModule,              // Debug module to validate, if available
    _COM_Outptr_ IDxcOperationResult **ppResult   // Validation output status, buffer, and errors
  );

  // IDxcValidator
  __override HRESULT STDMETHODCALLTYPE Validate(
    _In_ IDxcBlob *pShader,                       // Shader to validate.
    _In_ UINT32 Flags,                            // Validation flags.
    _COM_Outptr_ IDxcOperationResult **ppResult   // Validation output status, buffer, and errors
    );

  // IDxcVersionInfo
  __override HRESULT STDMETHODCALLTYPE GetVersion(_Out_ UINT32 *pMajor, _Out_ UINT32 *pMinor);
  __override HRESULT STDMETHODCALLTYPE GetFlags(_Out_ UINT32 *pFlags);
};

// Compile a single entry point to the target shader model
HRESULT STDMETHODCALLTYPE DxcValidator::Validate(
  _In_ IDxcBlob *pShader,                       // Shader to validate.
  _In_ UINT32 Flags,                            // Validation flags.
  _COM_Outptr_ IDxcOperationResult **ppResult   // Validation output status, buffer, and errors
) {
  if (pShader == nullptr || ppResult == nullptr || Flags & ~DxcValidatorFlags_ValidMask)
    return E_INVALIDARG;
  return ValidateWithOptModules(pShader, Flags, nullptr, nullptr, ppResult);
}

HRESULT DxcValidator::ValidateWithOptModules(
  _In_ IDxcBlob *pShader,                       // Shader to validate.
  _In_ UINT32 Flags,                            // Validation flags.
  _In_ llvm::Module *pModule,                   // Module to validate, if available.
  _In_ llvm::Module *pDebugModule,              // Debug module to validate, if available
  _COM_Outptr_ IDxcOperationResult **ppResult   // Validation output status, buffer, and errors
) {
  *ppResult = nullptr;
  HRESULT hr = S_OK;
  HRESULT validationStatus = S_OK;
  DxcEtw_DxcValidation_Start();
  try {
    CComPtr<IMalloc> pMalloc;
    CComPtr<AbstractMemoryStream> pDiagStream;
    IFT(CoGetMalloc(1, &pMalloc));
    IFT(CreateMemoryStream(pMalloc, &pDiagStream));

    // Run validation may throw, but that indicates an inability to validate,
    // not that the validation failed (eg out of memory).
    if (Flags & DxcValidatorFlags_RootSignatureOnly) {
      validationStatus = RunRootSignatureValidation(pShader, pDiagStream);
    } else {
      validationStatus = RunValidation(pShader, pModule, pDebugModule, pDiagStream);
    }
    if (FAILED(validationStatus)) {
      std::string msg("Validation failed.\n");
      ULONG cbWritten;
      pDiagStream->Write(msg.c_str(), msg.size(), &cbWritten);
    }
    // Assemble the result object.
    CComPtr<IDxcBlob> pDiagBlob;
    CComPtr<IDxcBlobEncoding> pDiagBlobEnconding;
    hr = pDiagStream.QueryInterface(&pDiagBlob);
    DXASSERT_NOMSG(SUCCEEDED(hr));
    IFT(DxcCreateBlobWithEncodingSet(pDiagBlob, CP_UTF8, &pDiagBlobEnconding));
    IFT(DxcOperationResult::CreateFromResultErrorStatus(nullptr, pDiagBlobEnconding, validationStatus, ppResult));
  }
  CATCH_CPP_ASSIGN_HRESULT();

  DxcEtw_DxcValidation_Stop(SUCCEEDED(hr) ? validationStatus : hr);
  return hr;
}

HRESULT STDMETHODCALLTYPE DxcValidator::GetVersion(_Out_ UINT32 *pMajor, _Out_ UINT32 *pMinor) {
  if (pMajor == nullptr || pMinor == nullptr)
    return E_INVALIDARG;
  GetValidationVersion(pMajor, pMinor);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE DxcValidator::GetFlags(_Out_ UINT32 *pFlags) {
  if (pFlags == nullptr)
    return E_INVALIDARG;
  *pFlags = DxcVersionInfoFlags_None;
#ifdef _NDEBUG
  *pFlags |= DxcVersionInfoFlags_Debug;
#endif
  return S_OK;
}

HRESULT DxcValidator::RunValidation(
  _In_ IDxcBlob *pShader,
  _In_ llvm::Module *pModule,                   // Module to validate, if available.
  _In_ llvm::Module *pDebugModule,              // Debug module to validate, if available
  _In_ AbstractMemoryStream *pDiagStream) {

  // Run validation may throw, but that indicates an inability to validate,
  // not that the validation failed (eg out of memory). That is indicated
  // by a failing HRESULT, and possibly error messages in the diagnostics stream.

  raw_stream_ostream DiagStream(pDiagStream);

  if (!pModule) {
    DXASSERT_NOMSG(pDebugModule == nullptr);
    if (IsDxilContainerLike(pShader->GetBufferPointer(), pShader->GetBufferSize())) {
      return ValidateDxilContainer(pShader->GetBufferPointer(), pShader->GetBufferSize(), DiagStream);
    } else {
      return ValidateDxilBitcode((const char*)pShader->GetBufferPointer(), (uint32_t)pShader->GetBufferSize(), DiagStream);
    }
  }

  llvm::DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
  PrintDiagnosticContext DiagContext(DiagPrinter);
  DiagRestore DR(pModule->getContext(), &DiagContext);

  IFR(hlsl::ValidateDxilModule(pModule, pDebugModule));
  IFR(ValidateDxilContainerParts(pModule, pDebugModule,
                    IsDxilContainerLike(pShader->GetBufferPointer(), pShader->GetBufferSize()),
                    (uint32_t)pShader->GetBufferSize()));

  if (DiagContext.HasErrors() || DiagContext.HasWarnings()) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  return S_OK;
}

HRESULT DxcValidator::RunRootSignatureValidation(
  _In_ IDxcBlob *pShader,
  _In_ AbstractMemoryStream *pDiagStream) {

  const DxilContainerHeader *pDxilContainer = IsDxilContainerLike(
    pShader->GetBufferPointer(), pShader->GetBufferSize());
  if (!pDxilContainer) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  const DxilProgramHeader *pProgramHeader = GetDxilProgramHeader(pDxilContainer, DFCC_DXIL);
  const DxilPartHeader *pPSVPart = GetDxilPartByType(pDxilContainer, DFCC_PipelineStateValidation);
  const DxilPartHeader *pRSPart = GetDxilPartByType(pDxilContainer, DFCC_RootSignature);
  IFRBOOL(pPSVPart && pRSPart, DXC_E_MISSING_PART);
  try {
    RootSignatureHandle RSH;
    RSH.LoadSerialized((const uint8_t*)GetDxilPartData(pRSPart), pRSPart->PartSize);
    RSH.Deserialize();
    raw_stream_ostream DiagStream(pDiagStream);
    IFRBOOL(VerifyRootSignatureWithShaderPSV(RSH.GetDesc(),
                                             GetVersionShaderType(pProgramHeader->ProgramVersion),
                                             GetDxilPartData(pPSVPart),
                                             pPSVPart->PartSize,
                                             DiagStream),
      DXC_E_INCORRECT_ROOT_SIGNATURE);
  } catch(...) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT RunInternalValidator(_In_ IDxcValidator *pValidator,
                             _In_ llvm::Module *pModule,
                             _In_ llvm::Module *pDebugModule,
                             _In_ IDxcBlob *pShader, UINT32 Flags,
                             _COM_Outptr_ IDxcOperationResult **ppResult) {
  DXASSERT_NOMSG(pValidator != nullptr);
  DXASSERT_NOMSG(pModule != nullptr);
  DXASSERT_NOMSG(pShader != nullptr);
  DXASSERT_NOMSG(ppResult != nullptr);

  DxcValidator *pInternalValidator = (DxcValidator *)pValidator;
  return pInternalValidator->ValidateWithOptModules(pShader, Flags, pModule,
                                                    pDebugModule, ppResult);
}

HRESULT CreateDxcValidator(_In_ REFIID riid, _Out_ LPVOID* ppv) {
  CComPtr<DxcValidator> result = new (std::nothrow) DxcValidator();
  if (result == nullptr) {
    *ppv = nullptr;
    return E_OUTOFMEMORY;
  }

  return result.p->QueryInterface(riid, ppv);
}

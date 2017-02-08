///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxc.cpp                                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the entry point for the dxc console program.                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// Some unimplemented flags as compared to fxc:
//
// /compress    - Compress DX10 shader bytecode from files.
// /decompress  - Decompress DX10 shader bytecode from first file.
// /Fx <file>   - Output assembly code and hex listing file.
// /Fl <file>   - Output a library.
// /Gch         - Compile as a child effect for fx_4_x profiles.
// /Gdp         - Disable effect performance mode.
// /Gec         - Enable backwards compatibility mode.
// /Ges         - Enable strict mode.
// /Gpp         - Force partial precision.
// /Lx          - Output hexadecimal literals
// /Op          - Disable preshaders
//
// Unimplemented but on roadmap:
//
// /matchUAVs   - Match template shader UAV slot allocations in the current shader
// /mergeUAVs   - Merge UAV slot allocations of template shader and the current shader
// /Ni          - Output instruction numbers in assembly listings
// /No          - Output instruction byte offset in assembly listings
// /Qstrip_reflect
// /res_may_alias
// /shtemplate
// /verifyrootsignature
//


#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"
#include <vector>
#include <string>

#include "dxc/dxcapi.h"
#include "dxc/dxcapi.internal.h"
#include "dxc/dxctools.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/HLSL/DxilContainer.h"
#include "dxc/HLSL/DxilShaderModel.h"
#include "dxc/HLSL/DxilRootSignature.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/microcom.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MemoryBuffer.h"
#include <dia2.h>
#include <comdef.h>
#include <algorithm>
#include <unordered_map>

inline bool wcseq(LPCWSTR a, LPCWSTR b) {
  return (a == nullptr && b == nullptr) || (a != nullptr && b != nullptr && wcscmp(a, b) == 0);
}
inline bool wcsieq(LPCWSTR a, LPCWSTR b) { return _wcsicmp(a, b) == 0; }

using namespace dxc;
using namespace llvm::opt;
using namespace hlsl::options;

class DxcContext {

private:
  DxcOpts &m_Opts;
  DxcDllSupport &m_dxcSupport;

  int ActOnBlob(IDxcBlob *pBlob);
  void UpdatePart(IDxcBlob *pBlob, IDxcBlob **ppResult);
  bool UpdatePartRequired();
  void WriteHeader(IDxcBlobEncoding *pDisassembly, IDxcBlob *pCode,
                   llvm::Twine &pVariableName, LPCWSTR pPath);
  HRESULT ReadFileIntoPartContent(hlsl::DxilFourCC fourCC, LPCWSTR fileName, IDxcBlob **ppResult);
  
  // TODO : Refactor two functions below. There are duplicate functions in DxcContext in dxa.cpp
  HRESULT GetDxcDiaTable(IDxcLibrary *pLibrary, IDxcBlob *pTargetBlob, IDiaTable **ppTable, LPCWSTR tableName);
  HRESULT FindModuleBlob(hlsl::DxilFourCC fourCC, IDxcBlob *pSource, IDxcLibrary *pLibrary, IDxcBlob **ppTargetBlob);
  void ExtractRootSignature(IDxcBlob *pBlob, IDxcBlob **ppResult);
  int VerifyRootSignature();

public:
  DxcContext(DxcOpts &Opts, DxcDllSupport &dxcSupport)
      : m_Opts(Opts), m_dxcSupport(dxcSupport) {}

  int  Compile();
  void Recompile(IDxcBlob *pSource, IDxcLibrary *pLibrary, IDxcCompiler *pCompiler, std::vector<LPCWSTR> &args, IDxcOperationResult **pCompileResult);
  int DumpBinary();
  void Preprocess();
};

static void WriteBlobToFile(_In_opt_ IDxcBlob *pBlob, llvm::StringRef FName) {
  ::dxc::WriteBlobToFile(pBlob, StringRefUtf16(FName));
}

static void WritePartToFile(IDxcBlob *pBlob, hlsl::DxilFourCC CC,
                            llvm::StringRef FName) {
  const hlsl::DxilContainerHeader *pContainer = hlsl::IsDxilContainerLike(
      pBlob->GetBufferPointer(), pBlob->GetBufferSize());
  if (!pContainer) {
    throw hlsl::Exception(E_FAIL, "Unable to find required part in blob");
  }
  hlsl::DxilPartIsType pred(CC);
  hlsl::DxilPartIterator it =
      std::find_if(hlsl::begin(pContainer), hlsl::end(pContainer), pred);
  if (it == hlsl::end(pContainer)) {
    throw hlsl::Exception(E_FAIL, "Unable to find required part in blob");
  }

  const char *pData = hlsl::GetDxilPartData(*it);
  DWORD dataLen = (*it)->PartSize;
  StringRefUtf16 WideName(FName);
  CHandle file(CreateFile2(WideName, GENERIC_WRITE, FILE_SHARE_READ,
                           CREATE_ALWAYS, nullptr));
  if (file == INVALID_HANDLE_VALUE) {
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), WideName);
  }
  DWORD written;
  if (FALSE == WriteFile(file, pData, dataLen, &written, nullptr)) {
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), WideName);
  }
}

// This function is called either after the compilation is done or /dumpbin option is provided
// Performing options that are used to process dxil container.
int DxcContext::ActOnBlob(IDxcBlob *pBlob) {
  int retVal = 0;
  // Text output.
  if (m_Opts.AstDump || m_Opts.OptDump) {
    WriteBlobToConsole(pBlob);
    return retVal;
  }

  // Write the output blob.
  if (!m_Opts.OutputObject.empty()) {
    // For backward compatability: fxc requires /Fo for /extractrootsignature
    if (!m_Opts.ExtractRootSignature) {
      CComPtr<IDxcBlob> pResult;
      UpdatePart(pBlob, &pResult);
      WriteBlobToFile(pResult, m_Opts.OutputObject);
    }
  }

  // Verify Root Signature
  if (!m_Opts.VerifyRootSignatureSource.empty()) {
    return VerifyRootSignature();
  }

  // Extract and write the PDB/debug information.
  if (!m_Opts.DebugFile.empty()) {
    IFTBOOLMSG(m_Opts.DebugInfo, E_INVALIDARG, "/Fd specified, but no Debug Info was "
      "found in the shader, please use the "
      "/Zi switch to generate debug "
      "information compiling this shader.");

    WritePartToFile(pBlob, hlsl::DFCC_ShaderDebugInfoDXIL, m_Opts.DebugFile);
  }

  // Extract and write root signature information.
  if (m_Opts.ExtractRootSignature) {
    CComPtr<IDxcBlob> pRootSignatureContainer;
    ExtractRootSignature(pBlob, &pRootSignatureContainer);
    WriteBlobToFile(pRootSignatureContainer, m_Opts.OutputObject);
  }

  // Extract and write private data.
  if (!m_Opts.ExtractPrivateFile.empty()) {
    WritePartToFile(pBlob, hlsl::DFCC_PrivateData, m_Opts.ExtractPrivateFile);
  }

  // OutputObject suppresses console dump.
  bool needDisassembly =
      !m_Opts.OutputHeader.empty() || !m_Opts.AssemblyCode.empty() ||
      (m_Opts.OutputObject.empty() && m_Opts.DebugFile.empty() &&
       m_Opts.ExtractPrivateFile.empty() &&
       m_Opts.VerifyRootSignatureSource.empty() && !m_Opts.ExtractRootSignature);

  if (!needDisassembly)
     return retVal;

  CComPtr<IDxcCompiler> pCompiler;
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));

  CComPtr<IDxcBlobEncoding> pDisassembleResult;
  IFT(pCompiler->Disassemble(pBlob, &pDisassembleResult));
  if (!m_Opts.OutputHeader.empty()) {
    llvm::Twine varName = m_Opts.VariableName.empty()
                              ? llvm::Twine("g_", m_Opts.EntryPoint)
                              : m_Opts.VariableName;
    WriteHeader(pDisassembleResult, pBlob, varName,
                StringRefUtf16(m_Opts.OutputHeader));
  } else if (!m_Opts.AssemblyCode.empty()) {
    WriteBlobToFile(pDisassembleResult, m_Opts.AssemblyCode);
  } else {
    WriteBlobToConsole(pDisassembleResult);
  }
  return retVal;
}

// Given a dxil container, update the dxil container by processing container specific options.
void DxcContext::UpdatePart(IDxcBlob *pSource, IDxcBlob **ppResult) {
  DXASSERT(pSource && ppResult, "otherwise blob cannot be updated");
  if (!UpdatePartRequired()) {
    *ppResult = pSource;
    pSource->AddRef();
    return;
  }

  CComPtr<IDxcContainerBuilder> pContainerBuilder;
  CComPtr<IDxcBlob> pResult;
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcContainerBuilder, &pContainerBuilder));
  
  // Load original container and update blob for each given option
  IFT(pContainerBuilder->Load(pSource));

  // Update parts based on dxc options
  if (m_Opts.StripDebug) {
    IFT(pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_ShaderDebugInfoDXIL));
  }
  if (m_Opts.StripPrivate) {
    IFT(pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_PrivateData));
  }
  if (m_Opts.StripRootSignature) {
    IFT(pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature));
  }
  if (!m_Opts.PrivateSource.empty()) {
    CComPtr<IDxcBlob> privateBlob;
    IFT(ReadFileIntoPartContent(hlsl::DxilFourCC::DFCC_PrivateData,
                                StringRefUtf16(m_Opts.PrivateSource),
                                &privateBlob));

    // setprivate option can replace existing private part. 
    // Try removing the private data if exists
    pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_PrivateData);
    IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_PrivateData, privateBlob));
  }
  if (!m_Opts.RootSignatureSource.empty()) {
    // set rootsignature assumes that the given input is a dxil container. 
    // We only want to add RTS0 part to the container builder. 
    CComPtr<IDxcBlob> RootSignatureBlob;
    IFT(ReadFileIntoPartContent(hlsl::DxilFourCC::DFCC_RootSignature,
                                StringRefUtf16(m_Opts.RootSignatureSource),
                                &RootSignatureBlob));

    // setrootsignature option can replace existing rootsignature part
    // Try removing rootsignature if exists
    pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature);
    IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_RootSignature, RootSignatureBlob));
  }
  
  // Get the final blob from container builder
  CComPtr<IDxcOperationResult> pBuilderResult;
  IFT(pContainerBuilder->SerializeContainer(&pBuilderResult));
  if (!m_Opts.OutputWarningsFile.empty()) {
    CComPtr<IDxcBlobEncoding> pErrors;
    IFT(pBuilderResult->GetErrorBuffer(&pErrors));
    if (pErrors != nullptr) {
      WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile);
    }
  }
  else {
    WriteOperationErrorsToConsole(pBuilderResult, m_Opts.OutputWarnings);
  }
  HRESULT status;
  IFT(pBuilderResult->GetStatus(&status));
  IFT(status);
  IFT(pBuilderResult->GetResult(ppResult));
}

bool DxcContext::UpdatePartRequired() {
  return m_Opts.StripDebug || m_Opts.StripPrivate ||
    m_Opts.StripRootSignature || !m_Opts.PrivateSource.empty() ||
    !m_Opts.RootSignatureSource.empty();
}

// This function reads the file from input file and constructs a blob with fourCC parts
// Used for setprivate and setrootsignature option
HRESULT DxcContext::ReadFileIntoPartContent(hlsl::DxilFourCC fourCC, LPCWSTR fileName, IDxcBlob **ppResult) {
  DXASSERT(fourCC == hlsl::DxilFourCC::DFCC_PrivateData ||
           fourCC == hlsl::DxilFourCC::DFCC_RootSignature,
           "Otherwise we provided wrong part to read for updating part.");

  // Read result, if it's private data, then return the blob
  if (fourCC == hlsl::DxilFourCC::DFCC_PrivateData) {
    CComPtr<IDxcBlobEncoding> pResult;
    ReadFileIntoBlob(m_dxcSupport, fileName, &pResult);
    *ppResult = pResult.Detach();
  }

  // If root signature, check if it's a dxil container that contains rootsignature part, then construct a blob of root signature part
  if (fourCC == hlsl::DxilFourCC::DFCC_RootSignature) {
    CComPtr<IDxcBlob> pResult;
    CComHeapPtr<BYTE> pData;
    DWORD dataSize;
    hlsl::ReadBinaryFile(fileName, (void**)&pData, &dataSize);
    DXASSERT(pData != nullptr, "otherwise ReadBinaryFile should throw an exception");
    hlsl::DxilContainerHeader *pHeader = (hlsl::DxilContainerHeader*) pData.m_pData;
    IFRBOOL(hlsl::IsDxilContainerLike(pHeader, pHeader->ContainerSizeInBytes), E_INVALIDARG);
    hlsl::DxilPartHeader *pPartHeader = hlsl::GetDxilPartByType(pHeader, hlsl::DxilFourCC::DFCC_RootSignature);
    IFRBOOL(pPartHeader != nullptr, E_INVALIDARG);
    hlsl::DxcCreateBlobOnHeapCopy(hlsl::GetDxilPartData(pPartHeader), pPartHeader->PartSize, &pResult);
    *ppResult = pResult.Detach();
  }
  return S_OK;
}

// Constructs a dxil container builder with only root signature part.
// Right now IDxcContainerBuilder assumes that we are building a full dxil container,
// but we are building a container with only rootsignature part
void DxcContext::ExtractRootSignature(IDxcBlob *pBlob, IDxcBlob **ppResult) {
  
  DXASSERT_NOMSG(pBlob != nullptr && ppResult != nullptr);
  const hlsl::DxilContainerHeader *pHeader = (hlsl::DxilContainerHeader *)(pBlob->GetBufferPointer());
  IFTBOOL(hlsl::IsValidDxilContainer(pHeader, pHeader->ContainerSizeInBytes), DXC_E_CONTAINER_INVALID);
  const hlsl::DxilPartHeader *pPartHeader = hlsl::GetDxilPartByType(pHeader, hlsl::DxilFourCC::DFCC_RootSignature);
  IFTBOOL(pPartHeader != nullptr, DXC_E_MISSING_PART);

  // Get new header and allocate memory for new container
  hlsl::DxilContainerHeader newHeader;
  uint32_t containerSize = hlsl::GetDxilContainerSizeFromParts(1, pPartHeader->PartSize);
  hlsl::InitDxilContainer(&newHeader, 1, containerSize); 
  CComPtr<IMalloc> pMalloc;
  CComPtr<hlsl::AbstractMemoryStream> pMemoryStream;
  IFT(CoGetMalloc(1, &pMalloc));
  IFT(hlsl::CreateMemoryStream(pMalloc, &pMemoryStream));
  ULONG cbWritten;

  // Write Container Header
  IFT(pMemoryStream->Write(&newHeader, sizeof(hlsl::DxilContainerHeader), &cbWritten));
  IFTBOOL(cbWritten == sizeof(hlsl::DxilContainerHeader), E_OUTOFMEMORY);
  
  // Write Part Offset
  uint32_t offset = sizeof(hlsl::DxilContainerHeader) + hlsl::GetOffsetTableSize(1);
  IFT(pMemoryStream->Write(&offset, sizeof(uint32_t), &cbWritten));
  IFTBOOL(cbWritten == sizeof(uint32_t), E_OUTOFMEMORY);
  
  // Write Root Signature Header
  IFT(pMemoryStream->Write(pPartHeader, sizeof(hlsl::DxilPartHeader), &cbWritten));
  IFTBOOL(cbWritten == sizeof(hlsl::DxilPartHeader), E_OUTOFMEMORY);
  const char * partContent = hlsl::GetDxilPartData(pPartHeader);
  
  // Write Root Signature Content
  IFT(pMemoryStream->Write(partContent, pPartHeader->PartSize, &cbWritten));
  IFTBOOL(cbWritten == pPartHeader->PartSize, E_OUTOFMEMORY);
  
  // Return Result
  CComPtr<IDxcBlob> pResult;
  IFT(pMemoryStream->QueryInterface(&pResult));
  *ppResult = pResult.Detach();
}

int DxcContext::VerifyRootSignature() {
  // Get dxil container from file
  CComPtr<IDxcBlobEncoding> pSource;
  ReadFileIntoBlob(m_dxcSupport, StringRefUtf16(m_Opts.InputFile), &pSource);
  hlsl::DxilContainerHeader *pSourceHeader = (hlsl::DxilContainerHeader *)pSource->GetBufferPointer();
  IFTBOOLMSG(hlsl::IsValidDxilContainer(pSourceHeader, pSourceHeader->ContainerSizeInBytes), E_INVALIDARG, "invalid DXIL container to verify.");

  // Get rootsignature from file
  CComPtr<IDxcBlob> pRootSignature;

  IFTMSG(ReadFileIntoPartContent(
             hlsl::DxilFourCC::DFCC_RootSignature,
             StringRefUtf16(m_Opts.VerifyRootSignatureSource), &pRootSignature),
         "invalid root signature to verify.");

  // TODO : Right now we are just going to bild a new blob with updated root signature to verify root signature
  // Since dxil container builder will verify on its behalf. 
  // This does unnecessary memory allocation. We can improve this later. 
  CComPtr<IDxcContainerBuilder> pContainerBuilder;
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcContainerBuilder, &pContainerBuilder));
  IFT(pContainerBuilder->Load(pSource));
  // Try removing root signature if it already exists
  pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature);
  IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_RootSignature, pRootSignature));  
  CComPtr<IDxcOperationResult> pOperationResult;
  IFT(pContainerBuilder->SerializeContainer(&pOperationResult));
  HRESULT status = E_FAIL;
  CComPtr<IDxcBlob> pResult;
  IFT(pOperationResult->GetStatus(&status));
  if (FAILED(status)) {
    if (!m_Opts.OutputWarningsFile.empty()) {
      CComPtr<IDxcBlobEncoding> pErrors;
      IFT(pOperationResult->GetErrorBuffer(&pErrors));
      WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile);
    }
    else {
      WriteOperationErrorsToConsole(pOperationResult, m_Opts.OutputWarnings);
    }
    return 1;
  }
  else {
    printf("root signature verification succeeded.");
    return 0;
  }
}

class DxcIncludeHandlerForInjectedSources : public IDxcIncludeHandler {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)

public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  DxcIncludeHandlerForInjectedSources() : m_dwRef(0) {};
  std::unordered_map<std::wstring, CComPtr<IDxcBlob>> includeFiles;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject) {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

  HRESULT insertIncludeFile(_In_ LPCWSTR pFilename, _In_ IDxcBlobEncoding *pBlob, _In_ UINT32 dataLen) {
    try {
      includeFiles.try_emplace(std::wstring(pFilename), pBlob);
    }
    CATCH_CPP_RETURN_HRESULT()
    return S_OK;
  }

  __override HRESULT STDMETHODCALLTYPE LoadSource(
    _In_ LPCWSTR pFilename,
    _COM_Outptr_result_maybenull_ IDxcBlob **ppIncludeSource
  ) {
    try {
      *ppIncludeSource = includeFiles.at(std::wstring(pFilename));
      (*ppIncludeSource)->AddRef();
    }
    CATCH_CPP_RETURN_HRESULT()
    return S_OK;
  }
};

void DxcContext::Recompile(IDxcBlob *pSource, IDxcLibrary *pLibrary, IDxcCompiler *pCompiler, std::vector<LPCWSTR> &args, IDxcOperationResult **ppCompileResult) {
  CComPtr<IDxcBlob> pTargetBlob;
  IFT(FindModuleBlob(hlsl::DxilFourCC::DFCC_ShaderDebugInfoDXIL, pSource, pLibrary, &pTargetBlob));
  // Retrieve necessary data from DIA symbols for recompiling
  CComPtr<IDiaTable> pSymbolTable;
  IFT(GetDxcDiaTable(pLibrary, pTargetBlob, &pSymbolTable, L"Symbols"));
  IFTARG(pSymbolTable != nullptr);
  std::wstring Arguments;   // Backing storage for blob arguments
  std::wstring BlobDefines; // Backing storage for blob defines
  std::string EntryPoint = m_Opts.EntryPoint;
  std::string TargetProfile = m_Opts.TargetProfile;
  std::vector<LPCWSTR> ConcatArgs;      // Blob arguments + command-line arguments
  std::vector<DxcDefine> ConcatDefines; // Blob defines + command-line defines
  CComBSTR pMainFileName;
  ULONG fetched;
  for (;;) {
    CComPtr<IUnknown> pSymbolUnk;
    CComPtr<IDiaSymbol> pSymbol;
    DWORD symTag;
    IFT(pSymbolTable->Next(1, &pSymbolUnk, &fetched));
    if (fetched == 0) {
      break;
    }
    IFT(pSymbolUnk->QueryInterface(&pSymbol));
    IFT(pSymbol->get_symTag(&symTag));

    CComBSTR pName;
    pSymbol->get_name(&pName);

    if (symTag == SymTagCompiland && wcseq(pName, L"main")) {
      pMainFileName.Empty();
      IFT(pSymbol->get_sourceFileName(&pMainFileName));
      continue;
    }

    // Check for well-known compiland environment values (name/value pairs).
    if (symTag != SymTagCompilandEnv) {
      continue;
    }

    CComVariant pVariant;
    IFT(pSymbol->get_value(&pVariant));
    if (EntryPoint.empty() && wcseq(pName, L"hlslEntry")) {
      IFTARG(VARENUM::VT_BSTR == pVariant.vt);
      EntryPoint = Unicode::UTF16ToUTF8StringOrThrow(pVariant.bstrVal);
      continue;
    }
    if (TargetProfile.empty() && wcseq(pName, L"hlslTarget")) {
      IFTARG(VARENUM::VT_BSTR == pVariant.vt);
      TargetProfile = Unicode::UTF16ToUTF8StringOrThrow(pVariant.bstrVal);
      continue;
    }
    if (wcseq(pName, L"hlslArguments")) {
      IFTARG(VARENUM::VT_BSTR == pVariant.vt);
      Arguments = pVariant.bstrVal;
      continue;
    }

    if (wcseq(pName, L"hlslDefines")) {
      IFTARG(VARENUM::VT_BSTR == pVariant.vt);
      // Parsing double null terminated string of defines into ConcatDefines.
      BlobDefines = std::wstring(pVariant.bstrVal, SysStringLen(pVariant.bstrVal));
      ConcatDefines.clear();
      LPWSTR Defines = const_cast<wchar_t *>(BlobDefines.data());
      UINT32 begin = 0;
      for (UINT32 i = 0;; ++i) {
        if (Defines[i] != L'\0') {
          continue;
        }
        if (begin != i) {
          wchar_t *pDefineStart = Defines + begin;
          wchar_t *pDefineEnd = Defines + i;
          DxcDefine D;
          D.Name = Defines + begin;
          D.Value = nullptr;
          for (wchar_t *pCursor = pDefineStart; pCursor < pDefineEnd+ i; ++pCursor) {
            if (*pCursor == L'=') {
              *pCursor = L'\0';
              D.Value = (pCursor + 1);
              break;
            }
          }
          ConcatDefines.push_back(D);
        }
        if (Defines[i + 1] == L'\0') {
          break;
        }
        begin = i + 1;
      }
    }
  }

  // Extracting file content from the blob
  CComPtr<IDiaTable> pInjectedSourceTable;
  IFT(GetDxcDiaTable(pLibrary, pTargetBlob, &pInjectedSourceTable, L"InjectedSource"));
  IFTARG(pInjectedSourceTable != nullptr);
  CComPtr<IDxcBlobEncoding> pCompileSource;
  CComPtr<DxcIncludeHandlerForInjectedSources> pIncludeHandler = new DxcIncludeHandlerForInjectedSources();
  for (;;) {
    CComPtr<IUnknown> pInjectedSourcesUnk;
    ULONG fetched;
    IFT(pInjectedSourceTable->Next(1, &pInjectedSourcesUnk, &fetched));
    if (fetched == 0) {
      break;
    }
    CComPtr<IDiaInjectedSource> pInjectedSource;
    IFT(pInjectedSourcesUnk->QueryInterface(&pInjectedSource));

    // Retrieve source from InjectedSource
    ULONGLONG dataLenLL;
    DWORD dataLen;
    IFT(pInjectedSource->get_length(&dataLenLL));
    IFT(ULongLongToDWord(dataLenLL, &dataLen));

    CComHeapPtr<BYTE> heapCopy;
    if (!heapCopy.AllocateBytes(dataLen)) {
      throw ::hlsl::Exception(E_OUTOFMEMORY);
    }
    CComPtr<IMalloc> pIMalloc;
    IFT(CoGetMalloc(1, &pIMalloc));
    IFT(pInjectedSource->get_source(dataLen, &dataLen, heapCopy.m_pData));

    // Get file name for this injected source
    CComBSTR pFileName;
    IFT(pInjectedSource->get_filename(&pFileName));
    IFTARG(pFileName);

    CComPtr<IDxcBlobEncoding> pBlobEncoding;
    IFT(pLibrary->CreateBlobWithEncodingOnMalloc(heapCopy.Detach(), pIMalloc, dataLen, CP_UTF8, &pBlobEncoding));
    IFT(pIncludeHandler->insertIncludeFile(pFileName, pBlobEncoding, dataLen));
    // Check if this file is the main file or included file
    if (wcscmp(pFileName, pMainFileName) == 0) {
      pCompileSource = pBlobEncoding.Detach();
    }
  }

  // Append arguments and defines from the command-line specification.
  for (LPCWSTR &A : args) {
    ConcatArgs.push_back(A);
  }
  for (DxcDefine &D : m_Opts.Defines.DefineVector) {
    ConcatDefines.push_back(D);
  }

  CComPtr<IDxcOperationResult> pResult;
  IFT(pCompiler->Compile(pCompileSource, pMainFileName,
    StringRefUtf16(EntryPoint),
    StringRefUtf16(TargetProfile), ConcatArgs.data(),
    ConcatArgs.size(), ConcatDefines.data(),
    ConcatDefines.size(), pIncludeHandler, &pResult));
  *ppCompileResult = pResult.Detach();
}

int DxcContext::Compile() {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pCompileResult;
  {
    CComPtr<IDxcBlobEncoding> pSource;

    std::vector<std::wstring> argStrings;
    CopyArgsToWStrings(m_Opts.Args, CoreOption, argStrings);

    std::vector<LPCWSTR> args;
    args.reserve(argStrings.size());
    for (const std::wstring &a : argStrings)
      args.push_back(a.data());

    if (m_Opts.AstDump)
      args.push_back(L"-ast-dump");

    CComPtr<IDxcLibrary> pLibrary;
    IFT(m_dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    IFT(m_dxcSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    ReadFileIntoBlob(m_dxcSupport, StringRefUtf16(m_Opts.InputFile), &pSource);
    IFTARG(pSource->GetBufferSize() >= 4);

    if (m_Opts.RecompileFromBinary) {
      Recompile(pSource, pLibrary, pCompiler, args, &pCompileResult);
    }
    else {
      CComPtr<IDxcIncludeHandler> pIncludeHandler;
      IFT(pLibrary->CreateIncludeHandler(&pIncludeHandler));

      // Upgrade profile to 6.0 version from minimum recognized shader model
      llvm::StringRef TargetProfile = m_Opts.TargetProfile;
      const hlsl::ShaderModel *SM = hlsl::ShaderModel::GetByName(m_Opts.TargetProfile.str().c_str());
      if (SM->IsValid() && SM->GetMajor() < 6) {
        TargetProfile = hlsl::ShaderModel::Get(SM->GetKind(), 6, 0)->GetName();
      }

      IFT(pCompiler->Compile(pSource, StringRefUtf16(m_Opts.InputFile),
        StringRefUtf16(m_Opts.EntryPoint),
        StringRefUtf16(TargetProfile), args.data(),
        args.size(), m_Opts.Defines.data(),
        m_Opts.Defines.size(), pIncludeHandler, &pCompileResult));
    }
  }

  if (!m_Opts.OutputWarningsFile.empty()) {
    CComPtr<IDxcBlobEncoding> pErrors;
    IFT(pCompileResult->GetErrorBuffer(&pErrors));
    WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile);
  }
  else {
    WriteOperationErrorsToConsole(pCompileResult, m_Opts.OutputWarnings);
  }

  HRESULT status;
  IFT(pCompileResult->GetStatus(&status));
  if (SUCCEEDED(status) || m_Opts.AstDump || m_Opts.OptDump) {
    CComPtr<IDxcBlob> pProgram;
    IFT(pCompileResult->GetResult(&pProgram));
    pCompiler.Release();
    pCompileResult.Release();
    if (pProgram.p != nullptr) {
      ActOnBlob(pProgram.p);
    }
  }
  return status;
}

int DxcContext::DumpBinary() {
  CComPtr<IDxcBlobEncoding> pSource;
  ReadFileIntoBlob(m_dxcSupport, StringRefUtf16(m_Opts.InputFile), &pSource);
  return ActOnBlob(pSource.p);
}

void DxcContext::Preprocess() {
  DXASSERT(!m_Opts.Preprocess.empty(), "else option reading should have failed");
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pPreprocessResult;
  CComPtr<IDxcBlobEncoding> pSource;
  std::vector<LPCWSTR> args;

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcIncludeHandler> pIncludeHandler;
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(pLibrary->CreateIncludeHandler(&pIncludeHandler));

  ReadFileIntoBlob(m_dxcSupport, StringRefUtf16(m_Opts.InputFile), &pSource);
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  IFT(pCompiler->Preprocess(pSource, StringRefUtf16(m_Opts.InputFile), args.data(), args.size(), m_Opts.Defines.data(), m_Opts.Defines.size(), pIncludeHandler, &pPreprocessResult));
  WriteOperationErrorsToConsole(pPreprocessResult, m_Opts.OutputWarnings);

  HRESULT status;
  IFT(pPreprocessResult->GetStatus(&status));
  if (SUCCEEDED(status)) {
    CComPtr<IDxcBlob> pProgram;
    IFT(pPreprocessResult->GetResult(&pProgram));
    WriteBlobToFile(pProgram, m_Opts.Preprocess);
  }
}

static void WriteString(HANDLE hFile, _In_z_ LPCSTR value, LPCWSTR pFileName) {
  DWORD written;
  if (FALSE == WriteFile(hFile, value, strlen(value) * sizeof(value[0]), &written, nullptr))
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), pFileName);
}

void DxcContext::WriteHeader(IDxcBlobEncoding *pDisassembly, IDxcBlob *pCode,
                             llvm::Twine &pVariableName, LPCWSTR pFileName) {
  CHandle file(CreateFile2(pFileName, GENERIC_WRITE, FILE_SHARE_READ,
                           CREATE_ALWAYS, nullptr));
  if (file == INVALID_HANDLE_VALUE) {
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), pFileName);
  }

  {
    std::string s;
    llvm::raw_string_ostream OS(s);
    // Note: with \r\n line endings, writing the disassembly could be a simple
    // WriteBlobToHandle with a prior and following WriteString for #ifs
    OS << "#if 0\r\n";
    const uint8_t *pBytes = (const uint8_t *)pDisassembly->GetBufferPointer();
    size_t len = pDisassembly->GetBufferSize();
    s.reserve(len + len * 0.1f); // rough estimate
    for (size_t i = 0; i < len; ++i) {
      if (pBytes[i] == '\n')
        OS << '\r';
      OS << pBytes[i];
    }
    OS << "\r\n#endif\r\n";
    OS.flush();
    WriteString(file, s.c_str(), pFileName);
  }

  {
    std::string s;
    llvm::raw_string_ostream OS(s);
    OS << "\r\nconst unsigned char " << pVariableName << "[] = {";
    const uint8_t *pBytes = (const uint8_t *)pCode->GetBufferPointer();
    size_t len = pCode->GetBufferSize();
    s.reserve(100 + len * 6 + (len / 12) * 3); // rough estimate
    for (size_t i = 0; i < len; ++i) {
      if (i != 0)
        OS << ',';
      if ((i % 12) == 0)
        OS << "\r\n ";
      OS << " 0x";
      if (pBytes[i] < 0x10)
        OS << '0';
      OS.write_hex(pBytes[i]);
    }
    OS << "\r\n};\r\n";
    OS.flush();
    WriteString(file, s.c_str(), pFileName);
  }
}

// Finds DXIL module from the blob assuming blob is either DxilContainer, DxilPartHeader, or DXIL module
HRESULT DxcContext::FindModuleBlob(hlsl::DxilFourCC fourCC, IDxcBlob *pSource, IDxcLibrary *pLibrary, IDxcBlob **ppTargetBlob) {
  if (!pSource || !pLibrary || !ppTargetBlob)
    return E_INVALIDARG;
  const UINT32 BC_C0DE = ((INT32)(INT8)'B' | (INT32)(INT8)'C' << 8 | (INT32)0xDEC0 << 16); // BC0xc0de in big endian
  if (BC_C0DE == *(UINT32*)pSource->GetBufferPointer()) {
    *ppTargetBlob = pSource;
    pSource->AddRef();
    return S_OK;
  }
  const char *pBitcode = nullptr;
  const hlsl::DxilPartHeader *pDxilPartHeader = (hlsl::DxilPartHeader*)pSource->GetBufferPointer(); // Initialize assuming that source is starting with DXIL part

  if (hlsl::IsValidDxilContainer((hlsl::DxilContainerHeader*)pSource->GetBufferPointer(), pSource->GetBufferSize())) {
    hlsl::DxilContainerHeader *pDxilContainerHeader = (hlsl::DxilContainerHeader*)pSource->GetBufferPointer();
    pDxilPartHeader = hlsl::GetDxilPartByType(pDxilContainerHeader, fourCC);
    IFTBOOL(pDxilPartHeader != nullptr, DXC_E_CONTAINER_MISSING_DEBUG);
  }
  if (fourCC == pDxilPartHeader->PartFourCC) {
    UINT32 pBlobSize;
    hlsl::DxilProgramHeader *pDxilProgramHeader = (hlsl::DxilProgramHeader*)(pDxilPartHeader + 1);
    hlsl::GetDxilProgramBitcode(pDxilProgramHeader, &pBitcode, &pBlobSize);
    UINT32 offset = (UINT32)(pBitcode - (const char *)pSource->GetBufferPointer());
    pLibrary->CreateBlobFromBlob(pSource, offset, pBlobSize, ppTargetBlob);
    return S_OK;
  }
  return E_INVALIDARG;
}

// TODO : There is an identical code in DxaContext in Dxa.cpp. Refactor this function.
HRESULT DxcContext::GetDxcDiaTable(IDxcLibrary *pLibrary, IDxcBlob *pTargetBlob, IDiaTable **ppTable, LPCWSTR tableName) {
  if (!pLibrary || !pTargetBlob || !ppTable)
    return E_INVALIDARG;
  CComPtr<IDiaDataSource> pDataSource;
  CComPtr<IStream> pSourceStream;
  CComPtr<IDiaSession> pSession;
  CComPtr<IDiaEnumTables> pEnumTables;
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDataSource));
  IFT(pLibrary->CreateStreamFromBlobReadOnly(pTargetBlob, &pSourceStream));
  IFT(pDataSource->loadDataFromIStream(pSourceStream));
  IFT(pDataSource->openSession(&pSession));
  IFT(pSession->getEnumTables(&pEnumTables));
  CComPtr<IDiaTable> pTable;
  for (;;) {
    ULONG fetched;
    pTable.Release();
    IFT(pEnumTables->Next(1, &pTable, &fetched));
    if (fetched == 0) {
      pTable.Release();
      break;
    }
    CComBSTR name;
    IFT(pTable->get_name(&name));
    if (wcscmp(name, tableName) == 0) {
      break;
    }
  }
  *ppTable = pTable.Detach();
  return S_OK;
}

int __cdecl wmain(int argc, const wchar_t **argv_) {
  const char *pStage = "Operation";
  int retVal = 0;
  try {
    pStage = "Argument processing";

    // Parse command line options.
    const OptTable *optionTable = getHlslOptTable();
    MainArgs argStrings(argc, argv_);
    DxcOpts dxcOpts;
    DxcDllSupport dxcSupport;

    // Read options and check errors.
    {
      std::string errorString;
      llvm::raw_string_ostream errorStream(errorString);
      int optResult =
          ReadDxcOpts(optionTable, DxcFlags, argStrings, dxcOpts, errorStream);
      errorStream.flush();
      if (errorString.size()) {
        fprintf(stderr, "dxc failed : %s", errorString.data());
      }
      if (optResult != 0) {
        return optResult;
      }
    }

    // Handle help request, which overrides any other processing.
    if (dxcOpts.ShowHelp) {
      std::string helpString;
      llvm::raw_string_ostream helpStream(helpString);
      optionTable->PrintHelp(helpStream, "dxc.exe", "HLSL Compiler");
      helpStream.flush();
      WriteUtf8ToConsoleSizeT(helpString.data(), helpString.size());
      return 0;
    }

    // Apply defaults.
    if (dxcOpts.EntryPoint.empty() && !dxcOpts.RecompileFromBinary) {
      dxcOpts.EntryPoint = "main";
    }

    // Setup a helper DLL.
    {
      std::string dllErrorString;
      llvm::raw_string_ostream dllErrorStream(dllErrorString);
      int dllResult = SetupDxcDllSupport(dxcOpts, dxcSupport, dllErrorStream);
      dllErrorStream.flush();
      if (dllErrorString.size()) {
        fprintf(stderr, "%s", dllErrorString.data());
      }
      if (dllResult)
        return dllResult;
    }

    EnsureEnabled(dxcSupport);
    DxcContext context(dxcOpts, dxcSupport);
    // TODO: implement all other actions.
    if (!dxcOpts.Preprocess.empty()) {
      pStage = "Preprocessing";
      context.Preprocess();
    }
    else if (dxcOpts.DumpBin) {
      pStage = "Dumping existing binary";
      retVal = context.DumpBinary();
    }
    else {
      pStage = "Compilation";
      retVal = context.Compile();
    }
  } catch (const ::hlsl::Exception &hlslException) {
    try {
      const char *msg = hlslException.what();
      Unicode::acp_char printBuffer[128]; // printBuffer is safe to treat as
                                          // UTF-8 because we use ASCII only errors
      if (msg == nullptr || *msg == '\0') {
        if (hlslException.hr == DXC_E_DUPLICATE_PART) {
          sprintf_s(
              printBuffer, _countof(printBuffer),
              "dxc failed : DXIL container already contains the given part.");
        } else if (hlslException.hr == DXC_E_MISSING_PART) {
          sprintf_s(
              printBuffer, _countof(printBuffer),
              "dxc failed : DXIL container does not contain the given part.");
        } else if (hlslException.hr == DXC_E_CONTAINER_INVALID) {
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : Invalid DXIL container.");
        } else if (hlslException.hr == DXC_E_CONTAINER_MISSING_DXIL) {
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : DXIL container is missing DXIL part.");
        } else if (hlslException.hr == DXC_E_CONTAINER_MISSING_DEBUG) {
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : DXIL container is missing Debug Info part.");
        } else if (hlslException.hr == E_OUTOFMEMORY) {
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : Out of Memory.");
        } else if (hlslException.hr == E_INVALIDARG) {
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : Invalid argument.");
        } else {
          sprintf_s(printBuffer, _countof(printBuffer),
            "dxc failed : error code 0x%08x.\n", hlslException.hr);
        }
        msg = printBuffer;
      }

      WriteUtf8ToConsoleSizeT(msg, strlen(msg), STD_ERROR_HANDLE);
      printf("\n");
    } catch (...) {
      printf("%s failed - unable to retrieve error message.\n", pStage);
    }

    return 1;
  } catch (std::bad_alloc &) {
    printf("%s failed - out of memory.\n", pStage);
    return 1;
  } catch (...) {
    printf("%s failed - unknown error.\n", pStage);
    return 1;
  }

  return retVal;
}

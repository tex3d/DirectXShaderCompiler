@echo off

if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

setlocal

if "%HLSL_SRC_DIR%"=="" (
  echo Missing source directory.
  if exist %~dp0..\..\LLVMBuild.txt (
    set HLSL_SRC_DIR=%~dp0..\..
    echo Source directory deduced to be %~dp0..\..
  ) else (
    exit /b 1
  )
)

if "%1"=="/buildoutdir" (
  echo Build output directory set to %2
  set HLSL_BLD_DIR=%2
  shift /1
  shift /1
)

if "%HLSL_BLD_DIR%"=="" (
  echo Missing build directory.
  exit /b 1
)

where cmake.exe 1>nul 2>nul
if errorlevel 1 (
  echo Unable to find cmake.exe on the path.
  echo cmake 3.4 is available from https://cmake.org/files/v3.4/cmake-3.4.0-win32-x86.exe
  exit /b 1
)

if "%BUILD_ARCH%"=="" (
  set BUILD_ARCH=Win32
)

rem MAXCPUCOUNT is for project-level parallelism
rem LLVM_COMPILER_JOBS is for object-level parallelism
rem You can set these in your environment to override these defaults.
rem The goal would be to find the best balance for this project on your build machine.
if "%MAXCPUCOUNT%"=="" set MAXCPUCOUNT=2
if "%LLVM_COMPILER_JOBS%"=="" set LLVM_COMPILER_JOBS=4

set BUILD_GENERATOR=Visual Studio 14 2015
set BUILD_VS_VER=2015
set BUILD_CONFIG=Debug
set DO_SETUP=1
set DO_BUILD=1
set CMAKE_OPTS=
if "%1"=="-s" (
  set DO_BUILD=0
  shift /1
)
if "%1"=="-b" (
  set DO_SETUP=0
  shift /1
)
if "%1"=="-alldef" (
  set CMAKE_OPTS=-DHLSL_OPTIONAL_PROJS_IN_DEFAULT:BOOL=ON
  shift /1
)
if "%1"=="-analyze" (
  set CMAKE_OPTS=%CMAKE_OPTS% -DHLSL_ENABLE_ANALYZE:BOOL=ON
  shift /1
)
if "%1"=="-fv" (
  echo Fixed version flag set for build.
  set CMAKE_OPTS=%CMAKE_OPTS% -DHLSL_ENABLE_FIXED_VER:BOOL=ON
  shift /1
)
if "%1"=="-cv" (
  echo Set the CLANG_VENDOR value.
  set CMAKE_OPTS=%CMAKE_OPTS% -DCLANG_VENDOR:STRING=%2
  shift /1
  shift /1
)
if "%1"=="-rel" (
  set BUILD_CONFIG=Release
  shift /1
)
if "%1"=="-x86" (
  set BUILD_ARCH=Win32
  shift /1
)
if "%1"=="-x64" (
  set BUILD_ARCH=x64
  shift /1
)
if "%1"=="-arm" (
  set BUILD_ARCH=ARM
  shift /1
)
if "%1"=="-Debug" (
  set BUILD_CONFIG=Debug
  shift /1
)
if "%1"=="-Release" (
  set BUILD_CONFIG=Release
  shift /1
)
if "%1"=="-vs2017" (
  set BUILD_GENERATOR=Visual Studio 15 2017
  set BUILD_VS_VER=2017
  shift /1
)

rem If only VS 2017 is available, pick that by default.
if "%BUILD_VS_VER%"=="2015" (
  reg query HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\DevDiv\vs\Servicing\14.0\devenv /v Install /reg:32 1>nul 2>nul
  if errorlevel 1 (
    echo Visual Studio 2015 not available, setting up build for Visual Studio 2017.
    set BUILD_GENERATOR=Visual Studio 15 2017
    set BUILD_VS_VER=2017
  )
)

if "%BUILD_ARCH%"=="x64" (
  set BUILD_GENERATOR=%BUILD_GENERATOR% %BUILD_ARCH:x64=Win64%
)

if "%1"=="-ninja" (
  set BUILD_GENERATOR=Ninja
  shift /1
)

set CMAKE_OPTS=%CMAKE_OPTS% -DCLANG_ENABLE_ARCMT:BOOL=OFF
set CMAKE_OPTS=%CMAKE_OPTS% -DCLANG_ENABLE_STATIC_ANALYZER:BOOL=OFF
set CMAKE_OPTS=%CMAKE_OPTS% -DCLANG_INCLUDE_TESTS:BOOL=OFF -DLLVM_INCLUDE_TESTS:BOOL=OFF
set CMAKE_OPTS=%CMAKE_OPTS% -DHLSL_INCLUDE_TESTS:BOOL=ON
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_TARGETS_TO_BUILD:STRING=None
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_INCLUDE_DOCS:BOOL=OFF -DLLVM_INCLUDE_EXAMPLES:BOOL=OFF
set CMAKE_OPTS=%CMAKE_OPTS% -DLIBCLANG_BUILD_STATIC:BOOL=ON
rem set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_OPTIMIZED_TABLEGEN:BOOL=ON
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_OPTIMIZED_TABLEGEN:BOOL=OFF
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_REQUIRES_EH:BOOL=ON
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_APPEND_VC_REV:BOOL=ON

rem Enable exception handling (which requires RTTI).
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_ENABLE_RTTI:BOOL=ON
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_ENABLE_EH:BOOL=ON

rem Setup a specific, stable triple for HLSL.
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_DEFAULT_TARGET_TRIPLE:STRING=dxil-ms-dx

set CMAKE_OPTS=%CMAKE_OPTS% -DCLANG_BUILD_EXAMPLES:BOOL=OFF
set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_REQUIRES_RTTI:BOOL=ON
set CMAKE_OPTS=%CMAKE_OPTS% -DCLANG_CL:BOOL=OFF
set CMAKE_OPTS=%CMAKE_OPTS% -DCMAKE_SYSTEM_VERSION=10.0.14393.0
set CMAKE_OPTS=%CMAKE_OPTS% -DDXC_BUILD_ARCH=%BUILD_ARCH%

set CMAKE_OPTS=%CMAKE_OPTS% -DLLVM_COMPILER_JOBS:STRING=%LLVM_COMPILER_JOBS%

rem This parameter is used with vcvarsall to force use of 64-bit build tools
rem instead of 32-bit tools that run out of memory.
if "%BUILD_ARCH%"=="Win32" (
  set BUILD_TOOLS=amd64_x86
) else if "%BUILD_ARCH%"=="x64" (
  set BUILD_TOOLS=amd64
) else if "%BUILD_ARCH%"=="ARM" (
  set BUILD_TOOLS=amd64_arm
)

call :configandbuild %BUILD_CONFIG% %BUILD_ARCH% %HLSL_BLD_DIR% "%BUILD_GENERATOR%"
if errorlevel 1 exit /b 1

if "%BUILD_GENERATOR%"=="Ninja" (
  echo Success - files are available at %HLSL_BLD_DIR%\bin
) else (
  echo Success - files are available at %HLSL_BLD_DIR%\%BUILD_CONFIG%\bin
)
call :handlesuccess
exit /b 0

:showhelp
echo Builds HLSL solutions and the product and test binaries for the current
echo flavor and architecture.
echo.
echo hctbuild [-s or -b] [-alldef] [-analyze] [-fv] [-rel] [-arm or -x86 or -x64] [-Release] [-Debug] [-vs2017] [-ninja]
echo.
echo   -s   creates the projects only, without building
echo   -b   builds the existing project
echo.
echo   -alldef  adds optional projects to the default build
echo   -analyze adds /analyze option
echo   -fv      fixes the resource version for release
echo.
echo   -rel builds release rather than debug
echo.
echo current BUILD_ARCH=%BUILD_ARCH%.  Override with:
echo   -x86 targets an x86 build (aka. Win32)
echo   -x64 targets an x64 build (aka. Win64)
echo   -arm targets an ARM build
echo.
echo Generator:
echo   -ninja   use Ninja as the generator
echo.
echo AppVeyor Support
echo   -Release builds release
echo   -Debug builds debug
echo   -vs2017 uses Visual Studio 2017 to build
echo.
if not "%HLSL_BLD_DIR%"=="" (
  echo The solution file is at %HLSL_BLD_DIR%\LLVM.sln
  echo.
)
goto :eof


:configandbuild
rem Configure and build a specific configuration, typically Debug or Release.
rem %1 - the conf name
rem %2 - the platform name
rem %3 - the build directory
rem %4 - the generator name
if not exist %3 (
  mkdir %3
  if errorlevel 1 (
    echo Unable to create %3
    call :handlefail
    exit /b 1
  )
)
cd /d %3
if "%DO_SETUP%"=="1" (
  echo Creating solution files for %2, logging to %3\cmake-log.txt
  if "%BUILD_GENERATOR%"=="Ninja" (
    echo Running cmake -DCMAKE_BUILD_TYPE:STRING=%1 %CMAKE_OPTS% -G %4 %HLSL_SRC_DIR% > %3\cmake-log.txt
    cmake -DCMAKE_BUILD_TYPE:STRING=%1 %CMAKE_OPTS% -G %4 %HLSL_SRC_DIR% >> %3\cmake-log.txt 2>&1
  ) else (
    rem -DCMAKE_BUILD_TYPE:STRING=%1 is not necessary for multi-config generators like VS
    echo Running cmake %CMAKE_OPTS% -G %4 %HLSL_SRC_DIR% > %3\cmake-log.txt
    cmake %CMAKE_OPTS% -G %4 %HLSL_SRC_DIR% >> %3\cmake-log.txt 2>&1
  )
  if errorlevel 1 (
    echo Failed to configure cmake projects.
    echo ===== begin cmake-log.txt =====
    type %3\cmake-log.txt
    echo ===== end cmake-log.txt =====
    echo Run 'cmake %HLSL_SRC_DIR%' in %3 will continue project generation after fixing the issue.
    cmake --version | findstr 3.4
    if errorlevel 1 (
      echo CMake 3.4 is the currently supported version - your installed cmake may be out of date.
      echo See README.md at the root for an explanation of dependencies.
    )
    findstr -c:"Could NOT find D3D12" %3\cmake-log.txt >NUL
    if errorlevel 1 (
      rem D3D12 has been found, nothing to diagnose here.
    ) else (
      echo D3D12 has not been found. Confirm that you have installed the Windows 10 SDK.
      echo See README.md at the root for an explanation of dependencies.
      echo Run hctclean after installing the SDK to completely rebuild the projects.
    )
    call :handlefail
    exit /b 1
  )
)

if "%DO_BUILD%"=="1" (
  rem Should add support for the non-x86-qualified programfiles.
  echo Building solution files for %2 with %1 configuration.
  if "%BUILD_GENERATOR%" NEQ "Ninja" (
    if "%BUILD_VS_VER%"=="2015" (
      if exist "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
        call :buildvs_x86dir %1 %2 %3
        goto :donebuild
      )
    )
  )

  rem Just defer to cmake for now.
  cmake --build . --config %1

:donebuild
  if errorlevel 1 (
    echo Failed to build projects.
    echo After fixing, run 'cmake --build --config %1 .' in %2
    call :handlefail
    exit /b 1
  )
  endlocal
)
exit /b 0

:buildvs_x86dir
rem Build with the VS tools in the x86 program files directory, maxcpucount makes this goes much faster.
rem 1 - config
rem 2 - platform
rem 3 - build directory
setlocal
call "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %BUILD_TOOLS%
rem Add /ds for a detailed summary at the end.
echo Logging to %3\msbuild-log.txt
MSBuild.exe /nologo /property:Configuration=%1 /property:Platform=%2 /maxcpucount:%MAXCPUCOUNT% %3\LLVM.sln /consoleloggerparameters:Summary;Verbosity=minimal /fileloggerparameters:LogFile=%3\msbuild-log.txt
if NOT "%ERRORLEVEL%"=="0" (
  exit /b 1
)
endlocal
exit /b 0

:handlefail
cscript.exe //Nologo %HLSL_SRC_DIR%\utils\hct\hctspeak.js /say:"build failed"
exit /b 0

:handlesuccess
cscript.exe //Nologo %HLSL_SRC_DIR%\utils\hct\hctspeak.js /say:"build succeeded"
exit /b 0

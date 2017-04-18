@echo off
setlocal ENABLEDELAYEDEXPANSION

if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

rem Whether we built the project using ninja as the generator.
set GENERATOR_NINJA=0

set TEST_ALL=1
set TEST_CLANG=0
set TEST_CMD=0
set TEST_EXEC=0
set TEST_EXTRAS=0
set TEST_EXEC_REQUIRED=0
set TEST_CLANG_FILTER= /select: "@Priority<1"
set TEST_EXEC_FILTER=ExecutionTest::*
if "%BUILD_CONFIG%"=="" (
  set BUILD_CONFIG=Debug
)
set HCT_DIR=%~dp0

:opt_loop
if "%1"=="" (goto :done_opt)

if "%1"=="-clean" (
  set TEST_CLEAN=1
) else if "%1"=="clean" (
  set TEST_CLEAN=1
  set TEST_CLEAN_ONLY=1
) else if "%1"=="none" (
  set TEST_ALL=0
) else if "%1"=="clang" (
  set TEST_ALL=0
  set TEST_CLANG=1
) else if "%1"=="clang-filter" (
  set TEST_ALL=0
  set TEST_CLANG=1
  set TEST_CLANG_FILTER= /name:%2
  shift /1
) else if "%1"=="v" (
  set TEST_ALL=0
  set TEST_CLANG=1
  set TEST_CLANG_FILTER= /name:VerifierTest::*
) else if "%1"=="cmd" (
  set TEST_ALL=0
  set TEST_CMD=1
) else if "%1"=="noexec" (
  set TEST_ALL=0
  set TEST_CLANG=1
  set TEST_CMD=1
) else if "%1"=="exec" (
  rem If exec is explicitly supplied, hcttest will fail if machine is not configured
  rem to run execution tests, otherwise, execution tests would be skipped.
  set TEST_ALL=0
  set TEST_EXEC=1
  set TEST_EXEC_REQUIRED=1
) else if "%1"=="exec-filter" (
  set TEST_ALL=0
  set TEST_EXEC=1
  set TEST_EXEC_FILTER=ExecutionTest::%2
  set TEST_EXEC_REQUIRED=1
  shift /1
) else if "%1"=="extras" (
  set TEST_ALL=0
  set TEST_EXTRAS=1
) else if "%1"=="-ninja" (
  set GENERATOR_NINJA=1
) else if "%1"=="-rel" (
  set BUILD_CONFIG=Release
) else if "%1"=="-x86" (
  rem Allow BUILD_ARCH override.  This may be used by HCT_EXTRAS scripts.
  set BUILD_ARCH=Win32
) else if "%1"=="-x64" (
  set BUILD_ARCH=x64
) else if "%1"=="-arm" (
  set BUILD_ARCH=ARM
) else if "%1"=="-adapter" (
  set TEST_ADAPTER= /p:"Adapter=%~2"
  shift /1
) else IF "%1"=="--" (
  shift /1
  goto :done_opt
) else (
  goto :done_opt
)
shift /1
goto :opt_loop
:done_opt

rem Collect additional arguments for tests
:collect_args
if "%1"=="" goto :done_args
set ADDITIONAL_OPTS=%ADDITIONAL_OPTS% %1
shift /1
goto :collect_args
:done_args

rem By default, run clang tests and execution tests.
if "%TEST_ALL%"=="1" (
  set TEST_CLANG=1
  set TEST_CMD=1
  set TEST_EXEC=1
  set TEST_EXTRAS=1
)

where te.exe 1>nul 2>nul
if errorlevel 1 (
  echo Unable to find te.exe on path.
  exit /b 1
)

Rem For the Ninja generator, artifacts are not generated into a directory
Rem matching the current build configuration; instead, they are generated
Rem directly into bin/ under the build root directory.
if "%GENERATOR_NINJA%"=="1" (
  set BIN_DIR=%HLSL_BLD_DIR%\bin
  set TEST_DIR=%HLSL_BLD_DIR%\test
) else (
  set BIN_DIR=%HLSL_BLD_DIR%\%BUILD_CONFIG%\bin
  set TEST_DIR=%HLSL_BLD_DIR%\%BUILD_CONFIG%\test
)

if "%TEST_CLEAN%"=="1" (
  echo Cleaning %TEST_DIR% ...
  if exist %TEST_DIR%\. (
    rmdir /q /s %TEST_DIR%
  )
  if "%TEST_CLEAN_ONLY%"=="1" (
    echo exiting after deleting test directory. if clean and test is desired, use -clean option.
    exit /b 0
  )
)

if not exist %TEST_DIR%\. (mkdir %TEST_DIR%)

echo Copying binaries to test to %TEST_DIR%:
call %HCT_DIR%\hctcopy.cmd %BIN_DIR% %TEST_DIR% dxa.exe dxc.exe dxexp.exe dxopt.exe dxr.exe dxv.exe clang-hlsl-tests.dll dxcompiler.dll d3dcompiler_dxc_bridge.dll
if errorlevel 1 exit /b 1

echo Running HLSL tests ...

if exist "%HCT_EXTRAS%\hcttest-before.cmd" (
  call "%HCT_EXTRAS%\hcttest-before.cmd" %TEST_DIR%
  if errorlevel 1 (
    echo Fatal error, Failed command: "%HCT_EXTRAS%\hcttest-before.cmd" %TEST_DIR%
    exit /b 1
  )
)

if "%TEST_CLANG%"=="1" (
  echo Running Clang unit tests ...
  call :runte clang-hlsl-tests.dll /p:"HlslDataDir=%HLSL_SRC_DIR%\tools\clang\test\HLSL"%TEST_CLANG_FILTER%%ADDITIONAL_OPTS%
  set RES_CLANG=!ERRORLEVEL!
)

if "%TEST_CMD%"=="1" (
  copy /y %HLSL_SRC_DIR%\utils\hct\smoke.hlsl %TEST_DIR%\smoke.hlsl
  call %HLSL_SRC_DIR%\utils\hct\hcttestcmds.cmd %TEST_DIR% %HLSL_SRC_DIR%\tools\clang\test\HLSL
  set RES_CMD=!ERRORLEVEL!
)

if "%TEST_EXEC%"=="1" (
  echo Sniffing for D3D12 configuration ...
  call :runte clang-hlsl-tests.dll /p:"HlslDataDir=%HLSL_SRC_DIR%\tools\clang\test\HLSL" /name:ExecutionTest::BasicTriangleTest /runIgnoredTests /p:"ExperimentalShaders=*" %TEST_ADAPTER%
  rem  /p:"ExperimentalShaders=*"
  set RES_EXEC=!ERRORLEVEL!
  if errorlevel 1 (
    if not "%TEST_EXEC_REQUIRED%"=="1" (
      echo Basic triangle test failed.
      echo Assuming this is an environmental limitation not a regression
      set TEST_EXEC=0
    ) else (
      echo Basic triangle test succeeded. Proceeding with execution tests.
    )
  )
)

if "%TEST_EXEC%"=="1" (
  call :runte clang-hlsl-tests.dll /p:"HlslDataDir=%HLSL_SRC_DIR%\tools\clang\test\HLSL" /name:%TEST_EXEC_FILTER% /runIgnoredTests /p:"ExperimentalShaders=*" %TEST_ADAPTER% %ADDITIONAL_OPTS%
  set RES_EXEC=!ERRORLEVEL!
)

if exist "%HCT_EXTRAS%\hcttest-extras.cmd" (
  if "%TEST_EXTRAS%"=="1" (
    echo Running extra tests ...
    call "%HCT_EXTRAS%\hcttest-extras.cmd" %TEST_DIR%
    set RES_EXTRAS=!ERRORLEVEL!
  )
)

if exist "%HCT_EXTRAS%\hcttest-after.cmd" (
  call "%HCT_EXTRAS%\hcttest-after.cmd" %TEST_DIR%
  set RES_HCTTEST_AFTER=!ERRORLEVEL!
)

echo.
echo ==================================
echo Unit test results:
set TESTS_PASSED=0
set TESTS_FAILED=0
call :check_result "clang tests" %RES_CLANG%
call :check_result "command line tests" %RES_CMD%
if "%TEST_EXEC_REQUIRED%"=="1" (
  call :check_result "execution tests" %RES_EXEC%
)
call :check_result "hcttest-extras tests" %RES_EXTRAS%
call :check_result "hcttest-after script" %RES_HCTTEST_AFTER%

if not "%TESTS_PASSED%"=="0" (
  echo %TESTS_PASSED% succeeded.
) else if "%TESTS_FAILED%"=="0" (
  echo No Unit tests run.
)
if not "%TESTS_FAILED%"=="0" (
  echo %TESTS_FAILED% failed.
)
echo ==================================
exit /b %TESTS_FAILED%

:showhelp

echo Usage:
echo   hcttest [options] [target(s)] [-- additonal test arguments]
echo.
echo target can be empty or a specific subset.
echo.
echo If target if not specified, all tests will be run, but clang tests
echo will be limited by /select: "@Priority<1" by default.
echo You may specify 'clang-filter *' to run all clang tests.
echo Multiple targets may be specified to choose which stages to run.
echo.
echo options:
echo   -clean - deletes test directory before copying binaries and testing
echo   -ninja - artifacts were built using the Ninja generator
echo   -rel   - builds release rather than debug
echo   -adapter "adapter name" - overrides Adapter for execution tests
echo.
echo current BUILD_ARCH=%BUILD_ARCH%.  Override with:
echo   -x86 targets an x86 build (aka. Win32)
echo   -x64 targets an x64 build (aka. Win64)
echo   -arm targets an ARM build
echo.
echo target(s):
echo  clang   - run clang tests.
echo  cmd     - run command line tool tests.
echo  v       - run the subset of clang tests that are verified-based.
echo  exec    - run execution tests.
echo  extras  - run hcttest-extras tests.
echo  noexec  - all except exec and extras tests.
echo.
echo Select clang or exec targets with filter by test name:
echo  clang-filter Name
echo  exec-filter Name
echo.
echo Use the HCT_EXTRAS environment variable to add hcttest-before and hcttest-after hooks.
echo.
echo Delete test directory and do not copy binaries or run tests:
echo   hcttest clean
echo.
call :showtesample clang-hlsl-tests.dll /p:"HlslDataDir=%HLSL_SRC_DIR%\tools\clang\test\HLSL"

goto :eof

:runte
rem Runs a unit test.
rem %1 - the name of the binary to run
rem %2 - first argument to te
rem %3 - second argument to te
rem %4 - third argument to te

set testerror=0
echo te /labMode /miniDumpOnCrash /logOutput:LowWithConsoleBuffering /parallel %TEST_DIR%\%* ^> %TEMP%\hcttest-taef.log
call te /labMode /miniDumpOnCrash /logOutput:LowWithConsoleBuffering /parallel %TEST_DIR%\%* > %TEMP%\hcttest-taef.log
set taeferror=%errorlevel%
type %TEMP%\hcttest-taef.log
if %taeferror% GEQ 65536 (
  echo TAEF test harness returned exit code: %taeferror%
  set testerror=%taeferror%
)
type %TEMP%\hcttest-taef.log | findstr /R /C:"^Summary: Total=[0-9][0-9]*, Passed=[0-9][0-9]*, Failed=[0-9][0-9]*, Blocked=[0-9][0-9]*, Not Run=[0-9][0-9]*, Skipped=[0-9][0-9]*" > %TEMP%\hcttest-taef-summary.log
if errorlevel 1 (
  echo Unable to parse Summary from TAEF log
  set testerror=%taeferror%
)
findstr /R /C:"Blocked=[1-9][0-9]*" %TEMP%\hcttest-taef-summary.log 1>nul 2>nul
if %errorlevel%==0 (
  echo Tests were Blocked
  set testerror=%taeferror%
)
findstr /R /C:"Failed=[1-9][0-9]*" %TEMP%\hcttest-taef-summary.log 1>nul 2>nul
if %errorlevel%==0 (
  echo Tests Failed
  set testerror=%taeferror%
)
if not %testerror%==0 (
  call :showtesample %*
)
exit /b %testerror%
goto :eof

:showtesample
rem %1 - name of binary to demo
rem %2 - first argument to te

echo You can debug the test with the following command line.
echo start devenv /debugexe TE.exe /inproc %TEST_DIR%\%*
echo.
echo Use this te.exe for out-of-proc, or pick the correct one for the target arch, currently x86.
where te.exe
echo.
echo Use /name:TestClass* or /name:TestClass::MethodName to filter.
goto :eof

:check_result
if not "%2"=="" (
  if "%2"=="0" (
    echo [PASSED] %~1
    set /a TESTS_PASSED=%TESTS_PASSED%+1
  ) else (
    echo [FAILED] %~1
    set /a TESTS_FAILED=%TESTS_FAILED%+1
  )
)
goto :eof

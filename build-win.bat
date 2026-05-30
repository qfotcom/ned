@echo off
setlocal EnableExtensions
echo Building NED for Windows...

REM Do NOT call VsDevCmd.bat here — it fails with "The input line is too long"
REM when PATH is already near the Windows limit. CMake's VS generator finds MSVC
REM and MSBuild automatically (same approach as build-win-ci.bat).

if defined VSCMD_VER (
    echo Using existing Visual Studio environment: %VSCMD_VER%
)

REM Resolve cmake.exe (PATH or bundled with Visual Studio)
set "CMAKE_EXE=cmake"
where cmake >nul 2>&1
if errorlevel 1 (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "%VSWHERE%" (
        for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`) do set "CMAKE_EXE=%%i"
    )
)
if /i not "%CMAKE_EXE%"=="cmake" (
    echo Using CMake: %CMAKE_EXE%
) else (
    where cmake >nul 2>&1
    if errorlevel 1 (
        echo CMake not found. Install CMake or Visual Studio with C++ Desktop Development.
        pause
        exit /b 1
    )
)

set VCPKG_ROOT=D:\APPLICATIONS\vcpkg
set VCPKG_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo vcpkg not found at %VCPKG_ROOT%
    echo Please install vcpkg or update VCPKG_ROOT in build-win.bat
    pause
    exit /b 1
)

echo Installing dependencies via vcpkg...
"%VCPKG_ROOT%\vcpkg.exe" install --triplet x64-windows
if %errorlevel% neq 0 (
    echo Failed to install vcpkg dependencies!
    exit /b 1
)

if not exist build mkdir build
cd build

echo Configuring with CMake...
"%CMAKE_EXE%" .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN_FILE%" -DVCPKG_TARGET_TRIPLET=x64-windows
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building project using %NUMBER_OF_PROCESSORS% CPU cores...
"%CMAKE_EXE%" --build . --config Release --parallel %NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo Executable should be in build\Release\ned.exe

echo Starting ned.exe...
.\Release\ned.exe

pause

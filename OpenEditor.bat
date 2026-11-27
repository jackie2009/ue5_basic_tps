@echo off
setlocal

:: Auto-elevate to Administrator via cmd.exe (works even if .bat lacks RunAs shell verb)
net session >nul 2>&1
if not "%ERRORLEVEL%"=="0" (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process -FilePath 'cmd.exe' -ArgumentList '/c','\"%~f0\"' -Verb RunAs"
    exit /b 0
)

set "PROJECT_DIR=%~dp0"
cd /d "%PROJECT_DIR%"
set "PROJECT=%PROJECT_DIR%basic_tps.uproject"
set "UE_ROOT=E:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR_EXE=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"

if not exist "%PROJECT%" (
    echo [ERROR] Project file not found: "%PROJECT%"
    pause
    exit /b 1
)

if not exist "%BUILD_BAT%" (
    echo [ERROR] Build.bat not found: "%BUILD_BAT%"
    pause
    exit /b 1
)

if not exist "%EDITOR_EXE%" (
    echo [ERROR] UnrealEditor.exe not found: "%EDITOR_EXE%"
    pause
    exit /b 1
)

echo [1/3] Closing any running UnrealEditor...
taskkill /F /IM UnrealEditor.exe >nul 2>&1
if "%ERRORLEVEL%"=="0" (
    timeout /t 2 /nobreak >nul
)

echo [2/3] Cleaning stale hot-reload files and building Development Editor...
del /F /Q "%PROJECT_DIR%Binaries\Win64\UnrealEditor-*-000*.*" >nul 2>&1
del /F /Q "%PROJECT_DIR%Binaries\Win64\UnrealEditor-*.patch_*.*" >nul 2>&1
del /F /Q "%PROJECT_DIR%Plugins\ItemSystem\Binaries\Win64\UnrealEditor-*-000*.*" >nul 2>&1
del /F /Q "%PROJECT_DIR%Intermediate\Build\Win64\x64\basic_tpsEditor\Development\HotReloadState.bin" >nul 2>&1

pushd "%UE_ROOT%\Engine\Build\BatchFiles"
call "Build.bat" basic_tpsEditor Win64 Development "-Project=%PROJECT%" -WaitMutex -architecture=x64
set "BUILD_RESULT=%ERRORLEVEL%"
popd

if not "%BUILD_RESULT%"=="0" (
    echo.
    echo [ERROR] Build failed with exit code %BUILD_RESULT%.
    pause
    exit /b %BUILD_RESULT%
)

echo [3/3] Launching Unreal Editor...
start "" /D "%PROJECT_DIR%" "%EDITOR_EXE%" "%PROJECT%"
exit /b 0
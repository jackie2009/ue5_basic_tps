@echo off
setlocal

set "PROJECT=%~dp0basic_tps.uproject"
set "BUILD_BAT=E:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"

if not exist "%PROJECT%" (
    echo ERROR: Project file not found: "%PROJECT%"
    pause
    exit /b 1
)

if not exist "%BUILD_BAT%" (
    echo ERROR: Unreal Engine build script not found: "%BUILD_BAT%"
    echo Update BUILD_BAT in this file if Unreal Engine was moved.
    pause
    exit /b 1
)

echo Building basic_tpsEditor Win64 Development...
call "%BUILD_BAT%" basic_tpsEditor Win64 Development "-Project=%PROJECT%" -WaitMutex -architecture=x64
set "BUILD_RESULT=%ERRORLEVEL%"

echo.
if "%BUILD_RESULT%"=="0" (
    echo Build succeeded. Restart the project editor to load the new module.
) else (
    echo Build failed with exit code %BUILD_RESULT%. See the output above.
)
pause
exit /b %BUILD_RESULT%

@echo off
setlocal enabledelayedexpansion

:: Path to glslangValidator (optional: add to PATH or specify full path here)
set GLSLANG=glslangValidator

:: Change working directory to this script's location
cd /d "%~dp0"

echo Current directory: %CD%
echo Starting shader compilation...
echo.

:: Initialize counters
set "success=0"
set "fail=0"

:: Compile each shader file
for %%F in (*.vert *.frag *.comp) do (
    echo Compiling %%F...

    %GLSLANG% -V "%%F" -o "%%F.spv"
    
    if errorlevel 1 (
        echo [ERROR] Failed to compile %%F
        set /a fail+=1
    ) else (
        echo [OK] Successfully compiled to %%F.spv
        set /a success+=1
    )

    echo.
)

echo Compilation Summary:
echo ---------------------
echo Successful: %success%
echo Failed    : %fail%

if %fail% NEQ 0 (
    echo.
    echo Some shaders failed to compile. Please check the errors above.
    pause
)

::pause

@echo off
REM Windows batch file wrapper for building NuGet package
REM Ensures PowerShell is available and calls the PowerShell script

where pwsh >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    pwsh -ExecutionPolicy Bypass -File "%~dp0build_package.ps1"
) else (
    where powershell >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        powershell -ExecutionPolicy Bypass -File "%~dp0build_package.ps1"
    ) else (
        echo ERROR: PowerShell is not available on this system.
        echo Please install PowerShell to run this script.
        exit /b 1
    )
)
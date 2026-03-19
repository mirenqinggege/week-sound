@echo off
REM Week Sound Auto-Start Uninstallation Script for Windows
REM This script removes week-sound from the user's auto-start programs

setlocal EnableDelayedExpansion

echo Uninstalling WeekSound auto-start program...

REM 获取可执行文件路径
set EXE_PATH=%~dp0..\build\bin\Release\week_sound.exe
if not exist "%EXE_PATH%" set EXE_PATH=%~dp0..\build\bin\week_sound.exe
if not exist "%EXE_PATH%" (
    echo Binary not found. Using registry removal only.
    REM 直接从注册表删除
    reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v WeekSound /f >nul 2>&1
    echo Auto-start entry removed from registry.
    exit /b 0
)

REM 使用程序自带的卸载功能
"%EXE_PATH%" -uninstall
if %errorLevel% neq 0 (
    echo Failed to remove auto-start entry, trying registry directly...
    reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v WeekSound /f >nul 2>&1
)

echo.
echo WeekSound auto-start uninstalled successfully!
echo.
echo Note: Config and log files are preserved. To remove them:
echo   rmdir /s "%ProgramData%\week-sound"

endlocal

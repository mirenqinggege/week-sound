@echo off
REM Week Sound Auto-Start Installation Script for Windows
REM This script adds week-sound to the user's auto-start programs
REM Note: Windows system services cannot access audio devices, so we use auto-start instead

setlocal EnableDelayedExpansion

set SERVICE_NAME=WeekSound
set INSTALL_DIR=%ProgramData%\week-sound
set CONFIG_FILE=%INSTALL_DIR%\week-sound.conf

echo Installing %SERVICE_NAME% as auto-start program...
echo Note: This is NOT a Windows service (services cannot access audio devices)
echo.

REM 创建目录
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

REM 复制配置文件
if exist "%~dp0..\config\week-sound.conf" (
    copy "%~dp0..\config\week-sound.conf" "%CONFIG_FILE%" >nul
) else (
    echo Creating default configuration...
    (
        echo [audio]
        echo frequency = 20
        echo amplitude = 0.01
        echo.
        echo [general]
        echo # log_file is auto-detected, uncomment to override
        echo #log_file =
    ) > "%CONFIG_FILE%"
)

REM 获取当前目录的可执行文件路径
set EXE_PATH=%~dp0..\build\bin\Release\week_sound.exe
if not exist "%EXE_PATH%" set EXE_PATH=%~dp0..\build\bin\week_sound.exe
if not exist "%EXE_PATH%" (
    echo Binary not found. Please build the project first.
    exit /b 1
)

REM 使用程序自带的安装功能
"%EXE_PATH%" -install
if %errorLevel% neq 0 (
    echo Failed to add auto-start entry
    exit /b 1
)

echo.
echo %SERVICE_NAME% auto-start installed successfully!
echo The program will start automatically when you log in.
echo.
echo To start now, run: "%EXE_PATH%"
echo.
echo Useful commands:
echo   sc query WeekSoundService    - NOT APPLICABLE (not a service)
echo   To uninstall: %EXE_PATH% -uninstall
echo.
echo Config file: %CONFIG_FILE%
echo Log file: %ProgramData%\week-sound\week-sound.log

endlocal

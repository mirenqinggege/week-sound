@echo off
REM Week Sound Service Installation Script for Windows
REM This script installs the week-sound service on Windows

setlocal EnableDelayedExpansion

set SERVICE_NAME=WeekSoundService
set SERVICE_DISPLAY_NAME=USB Soundcard Anti-Sleep Service
set INSTALL_DIR=%ProgramData%\week-sound
set CONFIG_FILE=%INSTALL_DIR%\week-sound.conf

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Please run as Administrator
    exit /b 1
)

echo Installing %SERVICE_NAME% service...

REM 创建目录
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

REM 复制配置文件
if exist "%~dp0..\config\week-sound.conf" (
    copy "%~dp0..\config\week-sound.conf" "%CONFIG_FILE%"
) else (
    echo Creating default configuration...
    (
        echo [audio]
        echo frequency = 20
        echo amplitude = 0.01
        echo.
        echo [general]
        echo log_file = %ProgramData%\week-sound\week-sound.log
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
    echo Failed to install service
    exit /b 1
)

REM 启动服务
sc start %SERVICE_NAME%

echo.
echo %SERVICE_NAME% service installed and started successfully!
echo.
echo Useful commands:
echo   sc query %SERVICE_NAME%    - Check service status
echo   sc stop %SERVICE_NAME%     - Stop service
echo   sc start %SERVICE_NAME%    - Start service

endlocal

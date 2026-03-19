@echo off
REM Week Sound Service Uninstallation Script for Windows
REM This script uninstalls the week-sound service from Windows

setlocal EnableDelayedExpansion

set SERVICE_NAME=WeekSoundService
set INSTALL_DIR=%ProgramData%\week-sound

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Please run as Administrator
    exit /b 1
)

echo Uninstalling %SERVICE_NAME% service...

REM 停止服务
sc stop %SERVICE_NAME% >nul 2>&1

REM 获取可执行文件路径
set EXE_PATH=%~dp0..\build\bin\Release\week_sound.exe
if not exist "%EXE_PATH%" set EXE_PATH=%~dp0..\build\bin\week_sound.exe

REM 使用程序自带的卸载功能
if exist "%EXE_PATH%" (
    "%EXE_PATH%" -uninstall
) else (
    REM 直接使用 sc 删除服务
    sc delete %SERVICE_NAME%
)

REM 询问是否删除配置
set /p REMOVE_CONFIG="Remove configuration directory? [y/N] "
if /i "%REMOVE_CONFIG%"=="y" (
    if exist "%INSTALL_DIR%" rmdir /s /q "%INSTALL_DIR%"
)

echo.
echo %SERVICE_NAME% service uninstalled successfully!

endlocal

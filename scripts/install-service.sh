#!/bin/bash

# Week Sound Service Installation Script
# This script installs the week-sound service as a user service (for PulseAudio support)

set -e

SERVICE_NAME="week-sound"
BINARY_NAME="week_sound"
CONFIG_DIR="${HOME}/.config/week-sound"
LOG_DIR="${HOME}/.local/share/week-sound/logs"
SERVICE_DIR="${HOME}/.config/systemd/user"
SERVICE_FILE="${SERVICE_DIR}/${SERVICE_NAME}.service"

echo "Installing ${SERVICE_NAME} as user service..."

# 创建目录
mkdir -p "${CONFIG_DIR}"
mkdir -p "${LOG_DIR}"
mkdir -p "${SERVICE_DIR}"

# 获取可执行文件的绝对路径
BINARY_PATH=""
if [ -f "./build/bin/${BINARY_NAME}" ]; then
    BINARY_PATH="$(realpath ./build/bin/${BINARY_NAME})"
elif [ -f "./bin/${BINARY_NAME}" ]; then
    BINARY_PATH="$(realpath ./bin/${BINARY_NAME})"
elif [ -f "${BINARY_NAME}" ]; then
    BINARY_PATH="$(realpath ${BINARY_NAME})"
else
    echo "Binary not found. Please build the project first."
    exit 1
fi

echo "Binary path: ${BINARY_PATH}"

# 复制配置文件（如果存在）
if [ -f "./config/week-sound.conf" ]; then
    cp "./config/week-sound.conf" "${CONFIG_DIR}/"
fi

# 创建 systemd 用户服务文件
cat > "${SERVICE_FILE}" << EOF
[Unit]
Description=USB Soundcard Anti-Sleep Service
After=pipewire-pulse.service pipewire.service
Wants=pipewire-pulse.service

[Service]
Type=simple
ExecStart=${BINARY_PATH}
Restart=on-failure
RestartSec=5
Environment=PATH=/usr/local/bin:/usr/bin:/bin

[Install]
WantedBy=default.target
EOF

# 创建日志目录
mkdir -p "${LOG_DIR}"

# 重新加载 systemd 用户服务
systemctl --user daemon-reload

# 启用并启动服务
systemctl --user enable ${SERVICE_NAME}
systemctl --user start ${SERVICE_NAME}

echo ""
echo "${SERVICE_NAME} user service installed and started successfully!"
echo ""
echo "Useful commands:"
echo "  systemctl --user status ${SERVICE_NAME}   - Check service status"
echo "  systemctl --user stop ${SERVICE_NAME}     - Stop service"
echo "  systemctl --user restart ${SERVICE_NAME}  - Restart service"
echo "  journalctl --user -u ${SERVICE_NAME} -f   - View logs"
echo ""
echo "Config file: ${CONFIG_DIR}/week-sound.conf"
echo "Log file: ${LOG_DIR}/week-sound.log"

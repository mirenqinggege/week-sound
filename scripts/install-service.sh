#!/bin/bash

# Week Sound Service Installation Script
# This script installs the week-sound service on Linux

set -e

SERVICE_NAME="week-sound"
BINARY_NAME="week_sound"
INSTALL_DIR="/usr/local/bin"
CONFIG_DIR="/etc/week-sound"
LOG_DIR="/var/log/week-sound"
SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"

# 检查是否以 root 运行
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (sudo)"
    exit 1
fi

echo "Installing ${SERVICE_NAME} service..."

# 创建目录
mkdir -p "${CONFIG_DIR}"
mkdir -p "${LOG_DIR}"

# 复制可执行文件
if [ -f "./build/bin/${BINARY_NAME}" ]; then
    cp "./build/bin/${BINARY_NAME}" "${INSTALL_DIR}/${SERVICE_NAME}"
    chmod +x "${INSTALL_DIR}/${SERVICE_NAME}"
elif [ -f "./bin/${BINARY_NAME}" ]; then
    cp "./bin/${BINARY_NAME}" "${INSTALL_DIR}/${SERVICE_NAME}"
    chmod +x "${INSTALL_DIR}/${SERVICE_NAME}"
else
    echo "Binary not found. Please build the project first."
    exit 1
fi

# 复制配置文件
cp "./config/week-sound.conf" "${CONFIG_DIR}/"

# 创建 systemd 服务文件
cat > "${SERVICE_FILE}" << EOF
[Unit]
Description=USB Soundcard Anti-Sleep Service
After=sound.target

[Service]
Type=simple
ExecStart=${INSTALL_DIR}/${SERVICE_NAME}
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

# 设置日志目录权限
chmod 755 "${LOG_DIR}"

# 重新加载 systemd
systemctl daemon-reload

# 启用并启动服务
systemctl enable ${SERVICE_NAME}
systemctl start ${SERVICE_NAME}

echo "${SERVICE_NAME} service installed and started successfully!"
echo ""
echo "Useful commands:"
echo "  systemctl status ${SERVICE_NAME}   - Check service status"
echo "  systemctl stop ${SERVICE_NAME}     - Stop service"
echo "  systemctl restart ${SERVICE_NAME}  - Restart service"
echo "  journalctl -u ${SERVICE_NAME} -f   - View logs"

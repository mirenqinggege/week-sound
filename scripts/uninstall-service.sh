#!/bin/bash

# Week Sound Service Uninstallation Script
# This script uninstalls the week-sound service from Linux

set -e

SERVICE_NAME="week-sound"
INSTALL_DIR="/usr/local/bin"
CONFIG_DIR="/etc/week-sound"
LOG_DIR="/var/log/week-sound"
SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"

# 检查是否以 root 运行
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (sudo)"
    exit 1
fi

echo "Uninstalling ${SERVICE_NAME} service..."

# 停止并禁用服务
if systemctl is-active --quiet ${SERVICE_NAME}; then
    systemctl stop ${SERVICE_NAME}
fi

if systemctl is-enabled --quiet ${SERVICE_NAME}; then
    systemctl disable ${SERVICE_NAME}
fi

# 删除服务文件
if [ -f "${SERVICE_FILE}" ]; then
    rm "${SERVICE_FILE}"
    systemctl daemon-reload
fi

# 删除可执行文件
if [ -f "${INSTALL_DIR}/${SERVICE_NAME}" ]; then
    rm "${INSTALL_DIR}/${SERVICE_NAME}"
fi

# 询问是否删除配置和日志
read -p "Remove configuration directory (${CONFIG_DIR})? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf "${CONFIG_DIR}"
fi

read -p "Remove log directory (${LOG_DIR})? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf "${LOG_DIR}"
fi

echo "${SERVICE_NAME} service uninstalled successfully!"

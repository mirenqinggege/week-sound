#!/bin/bash

# Week Sound Service Uninstallation Script
# This script uninstalls the week-sound user service

set -e

SERVICE_NAME="week-sound"
CONFIG_DIR="${HOME}/.config/week-sound"
LOG_DIR="${HOME}/.local/share/week-sound"
SERVICE_DIR="${HOME}/.config/systemd/user"
SERVICE_FILE="${SERVICE_DIR}/${SERVICE_NAME}.service"

echo "Uninstalling ${SERVICE_NAME} user service..."

# 停止并禁用服务
if systemctl --user is-active --quiet ${SERVICE_NAME} 2>/dev/null; then
    systemctl --user stop ${SERVICE_NAME}
fi

if systemctl --user is-enabled --quiet ${SERVICE_NAME} 2>/dev/null; then
    systemctl --user disable ${SERVICE_NAME}
fi

# 删除服务文件
if [ -f "${SERVICE_FILE}" ]; then
    rm "${SERVICE_FILE}"
    systemctl --user daemon-reload
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

echo "${SERVICE_NAME} user service uninstalled successfully!"

# Week Sound - USB 声卡防休眠服务

一个跨平台的 USB 声卡防休眠工具，通过持续输出低频声音（人耳几乎不可闻）来防止廉价 USB 声卡自动休眠。

## 功能特性

- 输出固定 20Hz 正弦波
- 极低音量（默认 1% 振幅），人耳几乎不可闻
- 自动跟随系统默认音频设备切换
- 设备不可用时自动重试
- 支持配置文件
- 日志记录
- 跨平台支持：
  - Linux：支持 systemd 服务
  - Windows：支持 Windows 服务

## 编译

### 依赖

- C++20 编译器
- CMake 3.16+

### Linux

```bash
# 安装依赖
sudo apt-get install -y libasound2-dev libpulse-dev

# 编译
mkdir -p build && cd build
cmake ..
make

# 可执行文件位于 build/bin/week_sound
```

### Windows 交叉编译（从 Linux）

```bash
# 安装 MinGW-w64
sudo apt-get install -y mingw-w64

# 编译
mkdir -p build-win64 && cd build-win64
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/win64-toolchain.cmake ..
make

# 可执行文件位于 build-win64/bin/week_sound.exe
```

### Windows 原生编译

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## 配置

配置文件位置：
- Linux: `/etc/week-sound/week-sound.conf`
- Windows: `C:\ProgramData\week-sound\week-sound.conf`

配置文件示例：

```ini
[audio]
frequency = 20          # 频率 (Hz)
amplitude = 0.01        # 振幅 (0.0-1.0)

[general]
log_file = /var/log/week-sound/week-sound.log
```

## 使用

### 直接运行

```bash
# Linux
./build/bin/week_sound

# Windows
.\build-win64\bin\week_sound.exe
```

### 安装为系统服务

#### Linux (systemd)

```bash
# 安装服务
sudo ./scripts/install-service.sh

# 管理服务
sudo systemctl status week-sound    # 查看状态
sudo systemctl stop week-sound      # 停止服务
sudo systemctl restart week-sound   # 重启服务

# 查看日志
sudo journalctl -u week-sound -f

# 卸载服务
sudo ./scripts/uninstall-service.sh
```

#### Windows 服务

```powershell
# 以管理员身份运行 PowerShell

# 安装服务
.\build-win64\bin\week_sound.exe -install

# 管理服务
sc query WeekSoundService      # 查看状态
sc stop WeekSoundService       # 停止服务
sc start WeekSoundService      # 启动服务

# 卸载服务
.\build-win64\bin\week_sound.exe -uninstall
```

或使用脚本：

```powershell
# 安装
.\scripts\install-service.bat

# 卸载
.\scripts\uninstall-service.bat
```

## 命令行参数

| 参数 | 说明 |
|-----|------|
| `-install` | 安装为 Windows 服务 |
| `-uninstall` | 卸载 Windows 服务 |
| `-h, --help` | 显示帮助信息 |

## 日志

日志文件位置：
- Linux: `/var/log/week-sound/week-sound.log`
- Windows: `C:\ProgramData\week-sound\week-sound.log`

日志格式：
```
[2026-03-19 14:30:00] [INFO] Service started
[2026-03-19 14:30:00] [INFO] Audio device opened: Default Device
[2026-03-19 14:30:00] [INFO] Playing 20Hz sine wave at 0.01 amplitude
```

## 项目结构

```
week-sound/
├── CMakeLists.txt              # 构建配置
├── cmake/
│   └── win64-toolchain.cmake   # Windows 交叉编译工具链
├── src/
│   ├── main.cpp                # 程序入口
│   ├── audio_player.cpp/h      # 音频播放核心
│   ├── config.cpp/h            # 配置文件解析
│   ├── logger.cpp/h            # 日志记录
│   ├── service_linux.cpp       # Linux 服务实现
│   └── service_windows.cpp     # Windows 服务实现
├── config/
│   └── week-sound.conf         # 默认配置文件
├── scripts/
│   ├── install-service.sh      # Linux 安装脚本
│   ├── uninstall-service.sh    # Linux 卸载脚本
│   ├── install-service.bat     # Windows 安装脚本
│   └── uninstall-service.bat   # Windows 卸载脚本
└── docs/
    └── plans/                  # 设计文档
```

## 技术细节

- **音频库**: RtAudio 6.0.1
- **音频 API**:
  - Linux: ALSA / PulseAudio
  - Windows: WASAPI
- **采样率**: 44100 Hz（自适应设备首选采样率）
- **缓冲区**: 256 帧

## 许可证

MIT License

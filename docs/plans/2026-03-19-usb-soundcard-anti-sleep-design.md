# USB 声卡防休眠服务设计文档

## 概述

实现一个持续输出低频声音的程序，防止 USB 声卡自动休眠。支持 Linux 和 Windows 双平台，可作为系统服务运行。

## 需求总结

- 输出固定 20Hz 正弦波
- 使用极低音量（人耳几乎不可闻）
- 输出到系统默认音频设备
- 支持 Linux systemd 服务
- 支持 Windows 服务
- 支持配置文件
- 记录日志到文件
- 自动跟随系统默认设备变化
- 设备不可用时持续重试

## 项目架构

```
week-sound/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # 程序入口，解析参数/服务管理
│   ├── audio_player.cpp/h    # 音频生成与输出核心逻辑
│   ├── config.cpp/h          # 配置文件解析
│   ├── logger.cpp/h          # 日志记录
│   ├── service_linux.cpp     # Linux systemd 服务相关
│   └── service_windows.cpp   # Windows 服务相关
├── config/
│   └── week-sound.conf       # 默认配置文件模板
├── scripts/
│   ├── install-service.sh    # Linux 安装服务脚本
│   └── install-service.bat   # Windows 安装服务脚本
└── README.md
```

## 依赖库

- **RtAudio**：跨平台音频输出库

## 核心模块设计

### 1. AudioPlayer（音频播放器）

**职责：** 生成 20Hz 正弦波并持续输出到系统默认音频设备

**实现细节：**
- 使用 RtAudio 打开默认输出设备
- 生成 20Hz 正弦波，采样率 44100Hz
- 振幅固定为极低值（默认 0.01，即 1%）
- 双缓冲机制确保连续输出
- 提供启动/停止接口

**波形生成公式：**
```cpp
sample = amplitude * sin(2 * PI * 20 * phase)
phase += 20.0 / sample_rate
```

**关键参数：**
- 采样率：44100 Hz
- 缓冲区大小：256 或 512 帧（由 RtAudio 建议）
- 声道：单声道或立体声（复制相同数据）
- 格式：32位浮点

### 2. Config（配置管理）

**职责：** 读取和管理运行参数

**配置文件格式（INI）：**
```ini
[audio]
frequency = 20
amplitude = 0.01      ; 音量比例 (0.0-1.0)

[general]
log_file = /var/log/week-sound/week-sound.log   ; Linux
; log_file = C:\ProgramData\week-sound\week-sound.log   ; Windows
```

**配置文件路径：**

| 平台 | 配置文件位置 |
|-----|-------------|
| Linux | `/etc/week-sound/week-sound.conf` |
| Windows | `C:\ProgramData\week-sound\week-sound.conf` |

### 3. Logger（日志记录）

**职责：** 记录运行状态到文件

**日志级别：** INFO、WARNING、ERROR

**日志格式：**
```
[2026-03-19 14:30:00] [INFO] Service started
[2026-03-19 14:30:00] [INFO] Audio device opened: Default Device
[2026-03-19 14:30:00] [INFO] Playing 20Hz sine wave at 0.01 amplitude
[2026-03-19 14:35:00] [WARNING] Audio buffer underrun, recovering...
[2026-03-19 15:00:00] [INFO] Service stopped
```

**记录事件：**
- 服务启动/停止
- 音频设备连接/断开/切换
- 错误信息

## 服务管理

### Linux 服务

**服务文件位置：** `/etc/systemd/system/week-sound.service`

**服务文件内容：**
```ini
[Unit]
Description=USB Soundcard Anti-Sleep Service
After=sound.target

[Service]
Type=simple
ExecStart=/usr/local/bin/week-sound
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

**安装脚本职责：**
- 复制可执行文件到 `/usr/local/bin/`
- 复制配置文件到 `/etc/week-sound/`
- 创建日志目录
- 启用并启动 systemd 服务

### Windows 服务

**实现方式：**
- 使用 Windows Service API (winsvc.h)
- 服务名称：`WeekSoundService`
- 显示名称：`USB Soundcard Anti-Sleep Service`

**命令行参数支持：**
- `-install`：安装服务
- `-uninstall`：卸载服务

**服务控制处理：**
- SERVICE_CONTROL_STOP：停止音频播放并退出
- SERVICE_CONTROL_PAUSE/CONTINUE：暂停/恢复音频

## 程序主流程

```
启动
  │
  ├── 检查命令行参数
  │     ├── -install    → 安装服务并退出
  │     ├── -uninstall  → 卸载服务并退出
  │     └── 无参数/其他 → 继续运行
  │
  ├── 加载配置文件
  │     └── 失败 → 使用默认配置 + 记录警告
  │
  ├── 初始化日志
  │     └── 失败 → 输出到 stderr
  │
  ├── 初始化音频播放器
  │     ├── 成功 → 开始播放
  │     └── 失败 → 记录错误，进入重试队列
  │
  └── 进入主循环
        ├── 定期检测设备变化（每 1 秒）
        │     └── 变化 → 重新初始化音频
        ├── 处理重试队列
        │     └── 有待重试项 → 尝试重新初始化
        └── 等待退出信号
              ├── Linux: SIGTERM/SIGINT
              └── Windows: 服务停止请求
```

## 设备变化处理

### 自动跟随流程

```
主循环中定期检测
  │
  ├── 获取当前默认设备 ID
  │
  ├── 与当前使用的设备 ID 比较
  │     ├── 相同 → 无操作
  │     └── 不同 → 触发设备切换
  │
  └── 设备切换流程
        ├── 关闭当前音频流
        ├── 打开新的默认设备
        │     └── 失败 → 记录错误，等待下次检测重试
        ├── 开始播放
        └── 记录日志
```

### 持续重试机制

| 情况 | 重试间隔 | 最大重试次数 |
|-----|---------|-------------|
| 设备打开失败 | 1 秒 | 无限制 |
| 音频流中断 | 立即重试 3 次，之后 5 秒 | 无限制 |

**重试期间行为：**
- 记录警告日志
- 不退出程序
- 服务状态保持运行

## 错误处理

| 错误场景 | 处理方式 |
|---------|---------|
| 配置文件不存在 | 使用默认配置，记录警告日志 |
| 配置文件格式错误 | 使用默认配置，记录警告日志 |
| 日志文件无法创建 | 输出到 stderr，继续运行 |
| 音频设备打开失败 | 记录错误，每 1 秒重试 |
| 音频播放中断 | 记录警告，尝试重新初始化设备 |
| 服务安装失败 | 输出错误信息，返回非零退出码 |

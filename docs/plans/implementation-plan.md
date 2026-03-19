# USB 声卡防休眠服务 - 实现计划

## 概述

基于设计文档实现跨平台的 USB 声卡防休眠服务。

## 前置条件

- C++20 编译器
- CMake 3.16+
- RtAudio 库

---

## 实现步骤

### 阶段一：项目基础设施

#### 步骤 1.1：更新 CMakeLists.txt
- 设置 C++20 标准
- 配置 RtAudio 依赖（使用 FetchContent 或 find_package）
- 配置平台特定编译选项
- 设置输出目录

#### 步骤 1.2：创建目录结构
```
src/
config/
scripts/
```

---

### 阶段二：核心模块实现

#### 步骤 2.1：实现 Logger 模块
**文件：** `src/logger.h`, `src/logger.cpp`

**功能：**
- 日志级别：INFO, WARNING, ERROR
- 写入文件（带时间戳格式）
- 支持日志文件路径配置
- 线程安全

**接口：**
```cpp
class Logger {
public:
    static Logger& instance();
    void init(const std::string& log_file);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
};
```

#### 步骤 2.2：实现 Config 模块
**文件：** `src/config.h`, `src/config.cpp`

**功能：**
- 解析 INI 格式配置文件
- 提供默认配置值
- 配置项：frequency, amplitude, log_file

**接口：**
```cpp
struct ConfigData {
    int frequency = 20;
    double amplitude = 0.01;
    std::string log_file;
};

class Config {
public:
    bool load(const std::string& config_path);
    const ConfigData& data() const;
    static std::string get_default_config_path();
};
```

#### 步骤 2.3：实现 AudioPlayer 模块
**文件：** `src/audio_player.h`, `src/audio_player.cpp`

**功能：**
- 使用 RtAudio 打开默认音频设备
- 生成 20Hz 正弦波
- 持续输出音频流
- 设备变化检测与自动切换
- 设备失败重试机制

**接口：**
```cpp
class AudioPlayer {
public:
    AudioPlayer(int frequency, double amplitude);
    ~AudioPlayer();

    bool start();
    void stop();

    bool is_playing() const;
    int current_device_id() const;

    // 检查设备是否变化，如有变化自动切换
    bool check_device_change();

    // 重试初始化
    bool retry_initialize();

private:
    RtAudio audio_;
    int frequency_;
    double amplitude_;
    int current_device_id_;
    bool is_playing_;
    double phase_;
};
```

**RtAudio 回调函数：**
```cpp
int audio_callback(void* output_buffer, void* input_buffer,
                   unsigned int n_buffer_frames,
                   double stream_time,
                   RtAudioStreamStatus status,
                   void* user_data);
```

---

### 阶段三：服务模块实现

#### 步骤 3.1：实现 Linux 服务模块
**文件：** `src/service_linux.cpp`

**功能：**
- 信号处理（SIGTERM, SIGINT）
- 服务安装/卸载脚本生成

**实现：**
- 使用 `signal()` 或 `sigaction()` 注册信号处理函数
- 主循环使用 `std::this_thread::sleep_for()` 定期检测设备变化

#### 步骤 3.2：实现 Windows 服务模块
**文件：** `src/service_windows.cpp`

**功能：**
- Windows 服务注册与控制
- 服务控制处理器（Service Control Handler）
- 命令行安装/卸载功能

**实现：**
- 使用 `RegisterServiceCtrlHandlerEx()`
- 使用 `SetServiceStatus()` 报告状态
- 使用 `CreateService()` / `DeleteService()` 安装/卸载

---

### 阶段四：主程序整合

#### 步骤 4.1：实现 main.cpp
**文件：** `src/main.cpp`

**功能：**
- 解析命令行参数
- 初始化各模块
- 主循环逻辑
- 优雅退出

**流程：**
1. 解析命令行参数（-install, -uninstall）
2. 加载配置文件
3. 初始化日志
4. 初始化音频播放器
5. 进入主循环：
   - 每 1 秒检测设备变化
   - 处理重试
6. 收到退出信号时停止并清理

---

### 阶段五：配置与脚本

#### 步骤 5.1：创建默认配置文件
**文件：** `config/week-sound.conf`

```ini
[audio]
frequency = 20
amplitude = 0.01

[general]
; Linux
log_file = /var/log/week-sound/week-sound.log
; Windows
; log_file = C:\ProgramData\week-sound\week-sound.log
```

#### 步骤 5.2：创建 Linux 安装脚本
**文件：** `scripts/install-service.sh`

**功能：**
- 创建目录（/etc/week-sound, /var/log/week-sound）
- 复制文件到系统目录
- 创建 systemd 服务文件
- 启用并启动服务

#### 步骤 5.3：创建 Windows 安装脚本
**文件：** `scripts/install-service.bat`

**功能：**
- 创建目录
- 复制文件
- 使用 sc 命令或程序自身 -install 参数安装服务

---

### 阶段六：测试与验证

#### 步骤 6.1：Linux 平台测试
- 编译程序
- 直接运行测试音频输出
- 测试服务安装
- 测试服务启停
- 测试设备切换

#### 步骤 6.2：Windows 平台测试
- 交叉编译或在 Windows 上编译
- 测试音频输出
- 测试服务安装
- 测试服务启停

---

## 文件清单

| 文件 | 阶段 | 说明 |
|-----|------|-----|
| CMakeLists.txt | 1.1 | 构建配置 |
| src/logger.h | 2.1 | 日志模块头文件 |
| src/logger.cpp | 2.1 | 日志模块实现 |
| src/config.h | 2.2 | 配置模块头文件 |
| src/config.cpp | 2.2 | 配置模块实现 |
| src/audio_player.h | 2.3 | 音频播放器头文件 |
| src/audio_player.cpp | 2.3 | 音频播放器实现 |
| src/service_linux.cpp | 3.1 | Linux 服务实现 |
| src/service_windows.cpp | 3.2 | Windows 服务实现 |
| src/main.cpp | 4.1 | 主程序 |
| config/week-sound.conf | 5.1 | 默认配置文件 |
| scripts/install-service.sh | 5.2 | Linux 安装脚本 |
| scripts/install-service.bat | 5.3 | Windows 安装脚本 |

---

## 构建命令

### Linux
```bash
mkdir build && cd build
cmake ..
make
```

### Windows
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

---

## 风险与注意事项

1. **RtAudio 依赖**：需要确保 RtAudio 正确链接，Linux 上需要 ALSA 开发库
2. **权限问题**：Linux 上访问音频设备可能需要用户在 audio 组
3. **Windows 服务**：需要管理员权限安装服务
4. **设备检测**：RtAudio 的设备 ID 可能在每次启动时变化，需要通过设备名称匹配

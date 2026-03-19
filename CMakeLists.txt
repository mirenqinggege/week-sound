cmake_minimum_required(VERSION 3.16)
project(week_sound VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# RtAudio 配置
if(WIN32)
    # Windows 使用 WASAPI
    set(RTAUDIO_API_WASAPI ON CACHE BOOL "" FORCE)
    set(RTAUDIO_API_DS OFF CACHE BOOL "" FORCE)
    set(RTAUDIO_API_ASIO OFF CACHE BOOL "" FORCE)
else()
    # Linux 使用 ALSA 和 PulseAudio
    set(RTAUDIO_API_ALSA ON CACHE BOOL "" FORCE)
    set(RTAUDIO_API_PULSE ON CACHE BOOL "" FORCE)
    set(RTAUDIO_API_JACK OFF CACHE BOOL "" FORCE)
endif()

# 禁用 RtAudio 示例程序
set(RTAUDIO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# RtAudio 依赖
include(FetchContent)
FetchContent_Declare(
    rtaudio
    GIT_REPOSITORY https://github.com/thestk/rtaudio.git
    GIT_TAG 6.0.1
)
FetchContent_MakeAvailable(rtaudio)

# 源文件
set(SOURCES
    src/main.cpp
    src/logger.cpp
    src/config.cpp
    src/audio_player.cpp
)

# 平台特定源文件
if(WIN32)
    list(APPEND SOURCES src/service_windows.cpp)
    add_definitions(-DWINDOWS_SERVICE)
else()
    list(APPEND SOURCES src/service_linux.cpp)
    add_definitions(-DLINUX_SERVICE)
endif()

# 可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE rtaudio)

# 平台特定设置
if(WIN32)
    # Windows 服务需要链接 advapi32 和其他库
    # MinGW 需要 pthread (winpthread)
    target_link_libraries(${PROJECT_NAME} PRIVATE advapi32 ole32 uuid pthread)
else()
    # Linux 需要 pthread
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

# 安装目标
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/config/week-sound.conf DESTINATION etc/week-sound)

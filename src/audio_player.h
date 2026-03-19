#ifndef WEEK_SOUND_AUDIO_PLAYER_H
#define WEEK_SOUND_AUDIO_PLAYER_H

#include <string>

class AudioPlayer {
public:
    AudioPlayer(int frequency, double amplitude);
    ~AudioPlayer();

    bool start();
    void stop();

    bool is_playing() const { return is_playing_; }
    int current_device_id() const { return current_device_id_; }

    // 检查设备是否变化，如有变化自动切换
    bool check_device_change();

    // 重试初始化
    bool retry_initialize();

    // 获取错误信息
    const std::string& last_error() const { return last_error_; }

private:
    bool initialize_audio();
    void cleanup_audio();

    int frequency_;
    double amplitude_;
    int current_device_id_;
    bool is_playing_;
    double phase_;

    void* rtaudio_;  // RtAudio* 使用 void* 避免头文件依赖
    std::string last_error_;

    static int audio_callback(void* output_buffer, void* input_buffer,
                              unsigned int n_buffer_frames,
                              double stream_time,
                              unsigned int stream_status,
                              void* user_data);
};

#endif // WEEK_SOUND_AUDIO_PLAYER_H

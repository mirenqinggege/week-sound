#include "audio_player.h"
#include "logger.h"
#include "RtAudio.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 存储采样率用于回调
static unsigned int g_sample_rate = 44100;

AudioPlayer::AudioPlayer(int frequency, double amplitude)
    : frequency_(frequency)
    , amplitude_(amplitude)
    , current_device_id_(-1)
    , is_playing_(false)
    , phase_(0.0)
    , rtaudio_(nullptr)
{
}

AudioPlayer::~AudioPlayer() {
    stop();
    cleanup_audio();
}

bool AudioPlayer::start() {
    if (is_playing_) {
        return true;
    }

    if (!initialize_audio()) {
        return false;
    }

    RtAudio* audio = static_cast<RtAudio*>(rtaudio_);
    try {
        audio->startStream();
        is_playing_ = true;
        Logger::instance().info("Audio playback started on device ID: " + std::to_string(current_device_id_));
        return true;
    } catch (std::exception& e) {
        last_error_ = e.what();
        Logger::instance().error("Failed to start audio stream: " + last_error_);
        return false;
    }
}

void AudioPlayer::stop() {
    if (!is_playing_) {
        return;
    }

    RtAudio* audio = static_cast<RtAudio*>(rtaudio_);
    if (audio && audio->isStreamOpen()) {
        try {
            audio->stopStream();
        } catch (std::exception& e) {
            Logger::instance().warning("Error stopping stream: " + std::string(e.what()));
        }
    }
    is_playing_ = false;
    Logger::instance().info("Audio playback stopped");
}

bool AudioPlayer::initialize_audio() {
    cleanup_audio();

    try {
        RtAudio* audio = new RtAudio();
        rtaudio_ = audio;

        // 获取默认输出设备 ID
        int default_device = audio->getDefaultOutputDevice();
        if (default_device < 0) {
            last_error_ = "No default output device available";
            Logger::instance().error(last_error_);
            return false;
        }

        RtAudio::StreamParameters parameters;
        parameters.deviceId = default_device;
        parameters.nChannels = 2;  // 立体声
        parameters.firstChannel = 0;

        // 获取设备信息以确定最佳采样率
        RtAudio::DeviceInfo info = audio->getDeviceInfo(default_device);
        unsigned int sample_rate = 44100;
        if (info.preferredSampleRate > 0) {
            sample_rate = info.preferredSampleRate;
        }
        g_sample_rate = sample_rate;

        unsigned int buffer_frames = 256;

        RtAudio::StreamOptions options;
        options.flags = RTAUDIO_MINIMIZE_LATENCY;

        audio->openStream(&parameters, nullptr, RTAUDIO_FLOAT32,
                         sample_rate, &buffer_frames,
                         &AudioPlayer::audio_callback, this, &options);

        current_device_id_ = default_device;
        phase_ = 0.0;

        Logger::instance().info("Audio device opened: " + info.name);
        Logger::instance().info("Playing " + std::to_string(frequency_) +
                               "Hz sine wave at " + std::to_string(amplitude_) + " amplitude");

        return true;
    } catch (std::exception& e) {
        last_error_ = e.what();
        Logger::instance().error("Failed to initialize audio: " + last_error_);
        cleanup_audio();
        return false;
    }
}

void AudioPlayer::cleanup_audio() {
    if (rtaudio_) {
        RtAudio* audio = static_cast<RtAudio*>(rtaudio_);
        if (audio->isStreamOpen()) {
            try {
                audio->closeStream();
            } catch (std::exception& e) {
                Logger::instance().warning("Error closing stream: " + std::string(e.what()));
            }
        }
        delete audio;
        rtaudio_ = nullptr;
    }
    current_device_id_ = -1;
}

bool AudioPlayer::check_device_change() {
    if (!rtaudio_) {
        return false;
    }

    RtAudio* audio = static_cast<RtAudio*>(rtaudio_);
    int new_default = audio->getDefaultOutputDevice();

    if (new_default != current_device_id_) {
        Logger::instance().info("Default audio device changed from " +
                               std::to_string(current_device_id_) + " to " +
                               std::to_string(new_default));

        bool was_playing = is_playing_;
        stop();
        cleanup_audio();

        if (was_playing) {
            return retry_initialize();
        }
        return true;
    }

    return false;
}

bool AudioPlayer::retry_initialize() {
    Logger::instance().info("Attempting to reinitialize audio...");
    return start();
}

int AudioPlayer::audio_callback(void* output_buffer, void* input_buffer,
                                unsigned int n_buffer_frames,
                                double stream_time,
                                unsigned int stream_status,
                                void* user_data)
{
    (void)input_buffer;
    (void)stream_time;
    (void)stream_status;

    AudioPlayer* player = static_cast<AudioPlayer*>(user_data);
    float* buffer = static_cast<float*>(output_buffer);

    double sample_rate = static_cast<double>(g_sample_rate);
    double phase_increment = 2.0 * M_PI * player->frequency_ / sample_rate;

    for (unsigned int i = 0; i < n_buffer_frames; i++) {
        float sample = static_cast<float>(player->amplitude_ * std::sin(player->phase_));

        // 立体声：左右声道相同
        buffer[i * 2] = sample;      // 左声道
        buffer[i * 2 + 1] = sample;  // 右声道

        player->phase_ += phase_increment;
        if (player->phase_ >= 2.0 * M_PI) {
            player->phase_ -= 2.0 * M_PI;
        }
    }

    return 0;
}

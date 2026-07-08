#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <string>
#include <thread>
#include <atomic>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#include <SDL.h>

class MusicPlayer {
public:
    enum class State {
        STOPPED,
        PLAYING,
        PAUSED
    };

    MusicPlayer();
    ~MusicPlayer();

    bool loadFile(const std::string& filename);
    bool play();
    bool pause();
    bool stop();
    bool seek(double seconds);
    
    void setVolume(float volume); // 0.0 to 1.0
    float getVolume() const;
    
    double getCurrentTime() const;
    double getDuration() const;
    State getState() const;
    
    std::string getCurrentFile() const;
    std::string getMetadata(const std::string& key) const;

private:
    // FFmpeg 核心组件
    AVFormatContext* m_formatContext;
    AVCodecContext* m_codecContext;
    SwrContext* m_swrContext;
    AVStream* m_audioStream;
    
    // SDL 音频组件
    SDL_AudioDeviceID m_audioDevice;
    SDL_AudioSpec m_audioSpec;
    
    // 播放状态控制（原子变量，线程安全）
    std::atomic<State> m_state;
    std::atomic<float> m_volume;
    std::atomic<double> m_currentTime;
    std::atomic<bool> m_seekRequested;
    std::atomic<double> m_seekTime;
    
    // 解码线程控制
    std::thread m_decodingThread;
    std::atomic<bool> m_shouldStop;
    
    // 当前文件元数据
    std::string m_currentFile;
    double m_duration;
    int m_audioStreamIndex;
    
    // 私有内部方法
    bool initializeFFmpeg();
    bool initializeSDL();
    void cleanup();
    void decodingLoop();
    
    bool setupAudioConversion();
    int decodeAudioFrame(AVFrame* frame, uint8_t** output, int* outputSize);
};

#endif // MUSICPLAYER_H
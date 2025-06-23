#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#include <SDL2/SDL.h>

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
    // FFmpeg components
    AVFormatContext* m_formatContext;
    AVCodecContext* m_codecContext;
    SwrContext* m_swrContext;
    AVStream* m_audioStream;
    
    // SDL Audio
    SDL_AudioDeviceID m_audioDevice;
    SDL_AudioSpec m_audioSpec;
    
    // Playback state
    std::atomic<State> m_state;
    std::atomic<float> m_volume;
    std::atomic<double> m_currentTime;
    std::atomic<bool> m_seekRequested;
    std::atomic<double> m_seekTime;
    
    // Threading
    std::thread m_decodingThread;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCondition;
    std::atomic<bool> m_shouldStop;
    
    // Audio buffer queue
    struct AudioBuffer {
        uint8_t* data;
        int size;
        double timestamp;
        
        AudioBuffer(int s) : size(s), timestamp(0) {
            data = new uint8_t[size];
        }
        
        ~AudioBuffer() {
            delete[] data;
        }
    };
    
    std::queue<std::unique_ptr<AudioBuffer>> m_audioQueue;
    static const size_t MAX_QUEUE_SIZE = 10;
    
    // File info
    std::string m_currentFile;
    double m_duration;
    int m_audioStreamIndex;
    
    // Private methods
    bool initializeFFmpeg();
    bool initializeSDL();
    void cleanup();
    void decodingLoop();
    
    static void audioCallback(void* userdata, uint8_t* stream, int len);
    void fillAudioBuffer(uint8_t* stream, int len);
    
    bool setupAudioConversion();
    int decodeAudioFrame(AVFrame* frame, uint8_t** output, int* outputSize);
};

#endif // MUSICPLAYER_H
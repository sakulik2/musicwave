#include "MusicPlayer.h"
#include <iostream>
#include <algorithm>
#include <cstring>

MusicPlayer::MusicPlayer() 
    : m_formatContext(nullptr)
    , m_codecContext(nullptr)
    , m_swrContext(nullptr)
    , m_audioStream(nullptr)
    , m_audioDevice(0)
    , m_state(State::STOPPED)
    , m_volume(1.0f)
    , m_currentTime(0.0)
    , m_seekRequested(false)
    , m_seekTime(0.0)
    , m_shouldStop(false)
    , m_duration(0.0)
    , m_audioStreamIndex(-1)
{
    initializeFFmpeg();
    initializeSDL();
}

MusicPlayer::~MusicPlayer() {
    stop();
    cleanup();
}

bool MusicPlayer::initializeFFmpeg() {
    // Set FFmpeg log level to reduce noise from MP3 timestamp warnings
    av_log_set_level(AV_LOG_WARNING);
    
    // Custom log callback to filter out specific warnings
    av_log_set_callback([](void* avcl, int level, const char* fmt, va_list vl) {
        // Skip the annoying MP3 timestamp warnings
        if (level <= AV_LOG_WARNING && fmt) {
            std::string msg(fmt);
            if (msg.find("Could not update timestamps for skipped samples") != std::string::npos ||
                msg.find("Could not update timestamps for discarded samples") != std::string::npos) {
                return; // Skip this warning
            }
        }
        
        // For other messages, use default behavior but only for errors
        if (level <= AV_LOG_ERROR) {
            vfprintf(stderr, fmt, vl);
        }
    });
    
    // Initialize FFmpeg network (av_register_all() is deprecated in FFmpeg 4.0+)
    avformat_network_init();
    return true;
}

bool MusicPlayer::initializeSDL() {
    // First, try to quit any existing SDL audio
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    
    // Try different initialization approaches
    std::vector<std::string> drivers = {"", "alsa", "pulse", "pipewire", "oss"};
    
    for (const auto& driver : drivers) {
        if (!driver.empty()) {
            std::cout << "Trying SDL audio driver: " << driver << std::endl;
            if (SDL_setenv("SDL_AUDIODRIVER", driver.c_str(), 1) != 0) {
                std::cerr << "Failed to set SDL_AUDIODRIVER" << std::endl;
                continue;
            }
        } else {
            std::cout << "Trying default SDL audio driver" << std::endl;
        }
        
        if (SDL_Init(SDL_INIT_AUDIO) >= 0) {
            std::cout << "SDL Audio initialized successfully with driver: " << 
                (SDL_GetCurrentAudioDriver() ? SDL_GetCurrentAudioDriver() : "Default") << std::endl;
            
            // Test if we can actually get audio devices
            int numDevices = SDL_GetNumAudioDevices(0);
            std::cout << "Found " << numDevices << " audio devices" << std::endl;
            
            if (numDevices > 0) {
                for (int i = 0; i < numDevices; i++) {
                    const char* deviceName = SDL_GetAudioDeviceName(i, 0);
                    std::cout << "  Device " << i << ": " << (deviceName ? deviceName : "Unknown") << std::endl;
                }
                return true; // Success!
            } else if (numDevices == 0) {
                std::cout << "No audio devices found with this driver, trying next..." << std::endl;
                SDL_QuitSubSystem(SDL_INIT_AUDIO);
                continue;
            }
        } else {
            std::cerr << "Failed to initialize SDL with driver " << (driver.empty() ? "default" : driver) 
                      << ": " << SDL_GetError() << std::endl;
        }
    }
    
    std::cerr << "Failed to initialize SDL audio with any driver!" << std::endl;
    std::cerr << "System audio troubleshooting:" << std::endl;
    std::cerr << "1. Run: aplay -l" << std::endl;
    std::cerr << "2. Test audio: speaker-test -c2 -t wav" << std::endl;
    std::cerr << "3. Check permissions: groups $USER | grep audio" << std::endl;
    std::cerr << "4. Install: sudo pacman -S alsa-utils pipewire-alsa" << std::endl;
    
    return false;
}

bool MusicPlayer::loadFile(const std::string& filename) {
    stop();
    cleanup();
    
    m_formatContext = avformat_alloc_context();
    if (!m_formatContext) {
        std::cerr << "Failed to allocate format context" << std::endl;
        return false;
    }
    
    // Open input file
    if (avformat_open_input(&m_formatContext, filename.c_str(), nullptr, nullptr) != 0) {
        std::cerr << "Failed to open input file: " << filename << std::endl;
        return false;
    }
    
    // Retrieve stream information
    if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
        std::cerr << "Failed to find stream information" << std::endl;
        return false;
    }
    
    // Find audio stream
    m_audioStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatContext->nb_streams; i++) {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIndex = i;
            break;
        }
    }
    
    if (m_audioStreamIndex == -1) {
        std::cerr << "No audio stream found" << std::endl;
        return false;
    }
    
    m_audioStream = m_formatContext->streams[m_audioStreamIndex];
    
    // Get codec
    const AVCodec* codec = avcodec_find_decoder(m_audioStream->codecpar->codec_id);
    if (!codec) {
        std::cerr << "Codec not found" << std::endl;
        return false;
    }
    
    // Allocate codec context
    m_codecContext = avcodec_alloc_context3(codec);
    if (!m_codecContext) {
        std::cerr << "Failed to allocate codec context" << std::endl;
        return false;
    }
    
    // Copy codec parameters
    if (avcodec_parameters_to_context(m_codecContext, m_audioStream->codecpar) < 0) {
        std::cerr << "Failed to copy codec parameters" << std::endl;
        return false;
    }
    
    // Open codec
    if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
        std::cerr << "Failed to open codec" << std::endl;
        return false;
    }
    
    // Calculate duration
    if (m_formatContext->duration != AV_NOPTS_VALUE) {
        m_duration = (double)m_formatContext->duration / AV_TIME_BASE;
    } else {
        m_duration = 0.0;
    }
    
    // Setup audio conversion
    if (!setupAudioConversion()) {
        return false;
    }
    
    m_currentFile = filename;
    return true;
}

bool MusicPlayer::setupAudioConversion() {
    // Setup SDL audio specification
    SDL_AudioSpec wanted, obtained;
    SDL_zero(wanted);
    wanted.freq = m_codecContext->sample_rate;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = m_codecContext->ch_layout.nb_channels;
    wanted.samples = 8192;  // Even smaller buffer for testing
    wanted.callback = nullptr;  // Use SDL_QueueAudio instead of callback
    wanted.userdata = nullptr;
    
    std::cout << "Requesting audio format:" << std::endl;
    std::cout << "  Sample rate: " << wanted.freq << " Hz" << std::endl;
    std::cout << "  Channels: " << (int)wanted.channels << std::endl;
    std::cout << "  Format: " << (wanted.format == AUDIO_S16SYS ? "16-bit signed" : "Other") << std::endl;
    
    // Get list of devices before trying to open
    int numDevices = SDL_GetNumAudioDevices(0);
    std::cout << "Found " << numDevices << " audio devices before opening:" << std::endl;
    
    std::vector<std::string> deviceNames;
    for (int i = 0; i < numDevices; i++) {
        const char* deviceName = SDL_GetAudioDeviceName(i, 0);
        if (deviceName) {
            deviceNames.push_back(std::string(deviceName));
            std::cout << "  Device " << i << ": " << deviceName << std::endl;
        }
    }
    
    // Try to open audio device with different approaches
    std::vector<Uint32> allowFlags = {
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE | SDL_AUDIO_ALLOW_FORMAT_CHANGE,
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE,
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE,
        0  // No changes allowed
    };
    
    // First try with default device (nullptr)
    for (auto flags : allowFlags) {
        std::cout << "Trying to open default audio device with flexibility flags: " << flags << std::endl;
        
        m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wanted, &obtained, flags);
        
        if (m_audioDevice != 0) {
            m_audioSpec = obtained;
            std::cout << "Default audio device opened successfully!" << std::endl;
            std::cout << "Actual audio format:" << std::endl;
            std::cout << "  Sample rate: " << m_audioSpec.freq << " Hz" << std::endl;
            std::cout << "  Channels: " << (int)m_audioSpec.channels << std::endl;
            std::cout << "  Buffer size: " << m_audioSpec.samples << " samples" << std::endl;
            break;
        } else {
            std::cout << "Failed with default device, flags " << flags << ": " << SDL_GetError() << std::endl;
        }
    }
    
    // If default device failed, try specific devices by name
    if (m_audioDevice == 0) {
        std::cout << "Default device failed, trying specific devices..." << std::endl;
        
        for (const auto& deviceName : deviceNames) {
            std::cout << "Trying device: " << deviceName << std::endl;
            
            for (auto flags : allowFlags) {
                m_audioDevice = SDL_OpenAudioDevice(deviceName.c_str(), 0, &wanted, &obtained, flags);
                
                if (m_audioDevice != 0) {
                    m_audioSpec = obtained;
                    std::cout << "Specific device '" << deviceName << "' opened successfully!" << std::endl;
                    std::cout << "Actual audio format:" << std::endl;
                    std::cout << "  Sample rate: " << m_audioSpec.freq << " Hz" << std::endl;
                    std::cout << "  Channels: " << (int)m_audioSpec.channels << std::endl;
                    std::cout << "  Buffer size: " << m_audioSpec.samples << " samples" << std::endl;
                    goto audio_success; // Break out of nested loops
                } else {
                    std::cout << "  Failed with flags " << flags << ": " << SDL_GetError() << std::endl;
                }
            }
        }
    }
    
    audio_success:
    
    if (m_audioDevice == 0) {
        std::cerr << "Failed to open audio device with any configuration!" << std::endl;
        
        // Try to reinitialize SDL audio as a last resort
        std::cerr << "Attempting to reinitialize SDL audio..." << std::endl;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        
        if (SDL_Init(SDL_INIT_AUDIO) >= 0) {
            std::cout << "SDL reinitialized, trying again..." << std::endl;
            
            // Try one more time with most flexible settings
            m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wanted, &obtained, 
                                               SDL_AUDIO_ALLOW_ANY_CHANGE);
            
            if (m_audioDevice != 0) {
                m_audioSpec = obtained;
                std::cout << "Success after reinitialization!" << std::endl;
            }
        }
    }
    
    if (m_audioDevice == 0) {
        std::cerr << "All audio device opening attempts failed!" << std::endl;
        std::cerr << "\nTry these manual solutions:" << std::endl;
        std::cerr << "1. Run: aplay -l  (check available devices)" << std::endl;
        std::cerr << "2. Run: SDL_AUDIODRIVER=alsa ./music_player" << std::endl;
        std::cerr << "3. Run: sudo pacman -S pipewire-alsa alsa-plugins" << std::endl;
        std::cerr << "4. Run: systemctl --user restart pipewire pipewire-pulse" << std::endl;
        
        return false;
    }
    
    // Setup resampling context
    m_swrContext = swr_alloc();
    if (!m_swrContext) {
        std::cerr << "Failed to allocate resampling context" << std::endl;
        return false;
    }
    
    // Create proper channel layout variables
    AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    AVChannelLayout in_ch_layout = m_codecContext->ch_layout;
    
    // Set resampling options for newer FFmpeg with channel layout support
    int ret = swr_alloc_set_opts2(&m_swrContext,
                                  &out_ch_layout,                    // out_ch_layout
                                  AV_SAMPLE_FMT_S16,                 // out_sample_fmt
                                  m_audioSpec.freq,                  // out_sample_rate
                                  &in_ch_layout,                     // in_ch_layout
                                  m_codecContext->sample_fmt,        // in_sample_fmt
                                  m_codecContext->sample_rate,       // in_sample_rate
                                  0, nullptr);
    
    if (ret < 0) {
        std::cerr << "Failed to configure resampling context" << std::endl;
        return false;
    }
    
    if (swr_init(m_swrContext) < 0) {
        std::cerr << "Failed to initialize resampling context" << std::endl;
        return false;
    }
    
    return true;
}

bool MusicPlayer::play() {
    if (m_state.load() == State::PLAYING) {
        return true;
    }
    
    if (m_currentFile.empty()) {
        return false;
    }
    
    if (m_state.load() == State::PAUSED) {
        m_state.store(State::PLAYING);
        SDL_PauseAudioDevice(m_audioDevice, 0);
        return true;
    }
    
    // Start decoding thread
    m_shouldStop.store(false);
    m_state.store(State::PLAYING);
    m_decodingThread = std::thread(&MusicPlayer::decodingLoop, this);
    
    // Start audio playback
    SDL_PauseAudioDevice(m_audioDevice, 0);
    
    return true;
}

bool MusicPlayer::pause() {
    if (m_state.load() == State::PLAYING) {
        m_state.store(State::PAUSED);
        SDL_PauseAudioDevice(m_audioDevice, 1);
        return true;
    }
    return false;
}

bool MusicPlayer::stop() {
    if (m_state.load() == State::STOPPED) {
        return true;
    }
    
    m_shouldStop.store(true);
    m_state.store(State::STOPPED);
    
    if (m_audioDevice) {
        SDL_PauseAudioDevice(m_audioDevice, 1);
        SDL_ClearQueuedAudio(m_audioDevice);
    }
    
    // Wake up decoding thread
    m_queueCondition.notify_all();
    
    if (m_decodingThread.joinable()) {
        m_decodingThread.join();
    }
    
    m_currentTime.store(0.0);
    return true;
}

bool MusicPlayer::seek(double seconds) {
    if (m_currentFile.empty()) {
        return false;
    }
    
    m_seekTime.store(seconds);
    m_seekRequested.store(true);
    return true;
}

void MusicPlayer::decodingLoop() {
    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    
    if (!frame) {
        std::cerr << "Failed to allocate frame in decoding loop" << std::endl;
        return;
    }
    
    std::cout << "Decoding thread started (using SDL_QueueAudio)" << std::endl;
    
    while (!m_shouldStop.load()) {
        // Handle seek requests
        if (m_seekRequested.load()) {
            int64_t seekTarget = (int64_t)(m_seekTime.load() * AV_TIME_BASE);
            av_seek_frame(m_formatContext, -1, seekTarget, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(m_codecContext);
            
            // Clear SDL audio queue
            SDL_ClearQueuedAudio(m_audioDevice);
            
            m_currentTime.store(m_seekTime.load());
            m_seekRequested.store(false);
        }
        
        // Check SDL audio queue size - don't let it get too full
        Uint32 queuedBytes = SDL_GetQueuedAudioSize(m_audioDevice);
        const Uint32 MAX_QUEUED_BYTES = 48000 * 2 * 2; // ~1 second of audio
        
        if (queuedBytes > MAX_QUEUED_BYTES) {
            // Queue is full, wait a bit
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        if (m_shouldStop.load()) break;
        
        // Read packet
        int ret = av_read_frame(m_formatContext, &packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                std::cout << "End of file reached" << std::endl;
                // Wait for audio queue to empty before stopping
                while (SDL_GetQueuedAudioSize(m_audioDevice) > 0 && !m_shouldStop.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                m_state.store(State::STOPPED);
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
        if (packet.stream_index == m_audioStreamIndex) {
            // Send packet to decoder
            ret = avcodec_send_packet(m_codecContext, &packet);
            if (ret < 0) {
                av_packet_unref(&packet);
                continue;
            }
            
            // Receive decoded frames
            while (ret >= 0 && !m_shouldStop.load()) {
                ret = avcodec_receive_frame(m_codecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    break;
                }
                
                // Convert audio format
                uint8_t* output = nullptr;
                int outputSize = 0;
                
                if (decodeAudioFrame(frame, &output, &outputSize) > 0) {
                    // Apply volume
                    float vol = m_volume.load();
                    if (vol < 0.99f) {
                        int16_t* samples = reinterpret_cast<int16_t*>(output);
                        int sampleCount = outputSize / sizeof(int16_t);
                        for (int i = 0; i < sampleCount; i++) {
                            samples[i] = static_cast<int16_t>(samples[i] * vol);
                        }
                    }
                    
                    // Queue audio data directly to SDL
                    if (SDL_QueueAudio(m_audioDevice, output, outputSize) < 0) {
                        std::cerr << "Failed to queue audio: " << SDL_GetError() << std::endl;
                    }
                    
                    // Update timestamp
                    double frameTime = 0.0;
                    if (packet.pts != AV_NOPTS_VALUE) {
                        frameTime = packet.pts * av_q2d(m_audioStream->time_base);
                    } else if (frame->pts != AV_NOPTS_VALUE) {
                        frameTime = frame->pts * av_q2d(m_audioStream->time_base);
                    } else {
                        // Estimate based on samples
                        double samplesPerSecond = m_codecContext->sample_rate;
                        double frameDuration = (double)frame->nb_samples / samplesPerSecond;
                        frameTime = m_currentTime.load() + frameDuration;
                    }
                    
                    m_currentTime.store(frameTime);
                    
                    av_free(output);
                    
                    static int frameCount = 0;
                    if (++frameCount % 100 == 0) {
                        std::cout << "Queued " << frameCount << " frames, SDL queue: " 
                                  << SDL_GetQueuedAudioSize(m_audioDevice) << " bytes" << std::endl;
                    }
                }
            }
        }
        
        av_packet_unref(&packet);
    }
    
    std::cout << "Decoding thread finished" << std::endl;
    av_frame_free(&frame);
}

int MusicPlayer::decodeAudioFrame(AVFrame* frame, uint8_t** output, int* outputSize) {
    int outputSamples = swr_get_out_samples(m_swrContext, frame->nb_samples);
    if (outputSamples <= 0) {
        return 0;
    }
    
    int outputBufferSize = av_samples_get_buffer_size(nullptr, m_audioSpec.channels, 
                                                      outputSamples, AV_SAMPLE_FMT_S16, 1);
    
    *output = (uint8_t*)av_malloc(outputBufferSize);
    if (!*output) {
        return 0;
    }
    
    int convertedSamples = swr_convert(m_swrContext, output, outputSamples,
                                      (const uint8_t**)frame->data, frame->nb_samples);
    
    if (convertedSamples < 0) {
        av_free(*output);
        return 0;
    }
    
    *outputSize = av_samples_get_buffer_size(nullptr, m_audioSpec.channels,
                                            convertedSamples, AV_SAMPLE_FMT_S16, 1);
    
    return convertedSamples;
}

// SDL_QueueAudio approach - no callback needed
void MusicPlayer::audioCallback(void* userdata, uint8_t* stream, int len) {
    // This function is no longer used with SDL_QueueAudio approach
}

void MusicPlayer::fillAudioBuffer(uint8_t* stream, int len) {
    // This function is no longer used with SDL_QueueAudio approach
}

void MusicPlayer::setVolume(float volume) {
    m_volume.store(std::max(0.0f, std::min(1.0f, volume)));
}

float MusicPlayer::getVolume() const {
    return m_volume.load();
}

double MusicPlayer::getCurrentTime() const {
    return m_currentTime.load();
}

double MusicPlayer::getDuration() const {
    return m_duration;
}

MusicPlayer::State MusicPlayer::getState() const {
    return m_state.load();
}

std::string MusicPlayer::getCurrentFile() const {
    return m_currentFile;
}

std::string MusicPlayer::getMetadata(const std::string& key) const {
    if (!m_formatContext) {
        return "";
    }
    
    AVDictionaryEntry* entry = av_dict_get(m_formatContext->metadata, key.c_str(), nullptr, 0);
    if (entry) {
        return std::string(entry->value);
    }
    
    return "";
}

void MusicPlayer::cleanup() {
    if (m_audioDevice) {
        SDL_CloseAudioDevice(m_audioDevice);
        m_audioDevice = 0;
    }
    
    if (m_swrContext) {
        swr_free(&m_swrContext);
    }
    
    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
    }
    
    if (m_formatContext) {
        avformat_close_input(&m_formatContext);
    }
    
    SDL_Quit();
}

#ifndef LTCWAVPLAYER_H
#define LTCWAVPLAYER_H

#include "WavReader.h"

#include <RtAudio.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct AsioDeviceEntry {
    unsigned int deviceId = 0;
    std::string name;
    unsigned int outputChannels = 0;
    unsigned int preferredSampleRate = 0;
};

class LtcWavPlayer {
public:
    using LogCallback = std::function<void(const std::string&)>;

    LtcWavPlayer();
    ~LtcWavPlayer();

    static std::vector<AsioDeviceEntry> listAsioDevices(std::string* error = nullptr);

    void setLogCallback(LogCallback callback);

    bool loadWav(const std::string& path, std::string* error = nullptr);
    bool start(unsigned int deviceId,
               unsigned int channelOffset,
               bool loop,
               std::string* error = nullptr);

    // Non-blocking: signals the audio callback to stop (safe on UI thread).
    void requestStop();
    // Blocking stop/close; call from a worker thread only.
    void stopBlocking();

    bool isPlaying() const;
    bool isStopRequested() const { return m_stopRequested.load(); }
    bool hasWav() const { return m_wav.frameCount() > 0; }

    unsigned int wavSampleRate() const { return m_wav.sampleRate(); }
    unsigned int wavChannels() const { return m_wav.channels(); }
    unsigned int wavFrameCount() const { return m_wav.frameCount(); }

private:
    void log(const std::string& message) const;

    WavReader m_wav;
    std::unique_ptr<RtAudio> m_audio;
    LogCallback m_logCallback;
    bool m_loop = true;
    std::atomic<bool> m_stopRequested{false};
    mutable std::mutex m_mutex;
};

#endif

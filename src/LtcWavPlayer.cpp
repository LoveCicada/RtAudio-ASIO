#include "LtcWavPlayer.h"

#include <algorithm>
#include <cstring>

namespace {

struct PlaybackState {
    const int16_t* samples = nullptr;
    unsigned int totalFrames = 0;
    unsigned int readFrame = 0;
    unsigned int wavChannels = 1;
    unsigned int outChannels = 1;
    unsigned int channelOffset = 0;
    bool loop = true;
    std::atomic<bool>* stopRequested = nullptr;
};

void errorCallback(RtAudioErrorType type, const std::string& errorText) {
    (void)type;
    (void)errorText;
}

int playbackCallback(void* outputBuffer,
                     void* /*inputBuffer*/,
                     unsigned int nBufferFrames,
                     double /*streamTime*/,
                     RtAudioStreamStatus status,
                     void* userData) {
    auto* state = static_cast<PlaybackState*>(userData);
    auto* out = static_cast<int16_t*>(outputBuffer);

    if (status) {
        // underflow, etc.
    }

    if (state && state->stopRequested && state->stopRequested->load()) {
        return 2;
    }

    if (!state || !state->samples || state->totalFrames == 0) {
        const unsigned int ch = state ? state->outChannels : 1;
        std::memset(out, 0, nBufferFrames * ch * sizeof(int16_t));
        return 0;
    }

    unsigned int framesWritten = 0;
    while (framesWritten < nBufferFrames) {
        if (state->readFrame >= state->totalFrames) {
            if (!state->loop) {
                const unsigned int remaining = nBufferFrames - framesWritten;
                std::memset(out + framesWritten * state->outChannels,
                            0,
                            remaining * state->outChannels * sizeof(int16_t));
                return 1;
            }
            state->readFrame = 0;
        }

        const unsigned int srcFramesLeft = state->totalFrames - state->readFrame;
        const unsigned int framesToCopy =
            std::min(nBufferFrames - framesWritten, srcFramesLeft);

        for (unsigned int f = 0; f < framesToCopy; ++f) {
            const unsigned int srcFrame = state->readFrame + f;
            int16_t left = 0;
            int16_t right = 0;

            if (state->wavChannels == 1) {
                left = state->samples[srcFrame];
                right = left;
            } else {
                left = state->samples[srcFrame * 2];
                right = state->samples[srcFrame * 2 + 1];
            }

            int16_t* frameOut = out + (framesWritten + f) * state->outChannels;
            for (unsigned int ch = 0; ch < state->outChannels; ++ch) {
                if (state->wavChannels >= 2 && ch == 1) {
                    frameOut[ch] = right;
                } else {
                    frameOut[ch] = left;
                }
            }
        }

        state->readFrame += framesToCopy;
        framesWritten += framesToCopy;
    }

    return 0;
}

} // namespace

LtcWavPlayer::LtcWavPlayer() = default;

LtcWavPlayer::~LtcWavPlayer() {
    stopBlocking();
}

void LtcWavPlayer::setLogCallback(LogCallback callback) {
    m_logCallback = std::move(callback);
}

void LtcWavPlayer::log(const std::string& message) const {
    if (m_logCallback) {
        m_logCallback(message);
    }
}

std::vector<AsioDeviceEntry> LtcWavPlayer::listAsioDevices(std::string* error) {
    std::vector<AsioDeviceEntry> devices;
    RtAudio audio(RtAudio::WINDOWS_ASIO, errorCallback);
    if (audio.getCurrentApi() != RtAudio::WINDOWS_ASIO) {
        if (error) {
            *error = "RtAudio ASIO API is not available in this build.";
        }
        return devices;
    }

    const std::vector<unsigned int> ids = audio.getDeviceIds();
    for (unsigned int id : ids) {
        const RtAudio::DeviceInfo info = audio.getDeviceInfo(id);
        if (info.outputChannels == 0) {
            continue;
        }
        AsioDeviceEntry entry;
        entry.deviceId = id;
        entry.name = info.name;
        entry.outputChannels = info.outputChannels;
        entry.preferredSampleRate =
            info.preferredSampleRate != 0 ? info.preferredSampleRate : info.currentSampleRate;
        devices.push_back(entry);
    }
    return devices;
}

bool LtcWavPlayer::loadWav(const std::string& path, std::string* error) {
    requestStop();
    stopBlocking();
    return m_wav.load(path, error);
}

bool LtcWavPlayer::start(unsigned int deviceId,
                         unsigned int channelOffset,
                         bool loop,
                         std::string* error) {
    requestStop();
    stopBlocking();

    if (m_wav.frameCount() == 0) {
        if (error) {
            *error = "No WAV loaded";
        }
        return false;
    }

    m_loop = loop;
    m_stopRequested.store(false);

    std::lock_guard<std::mutex> lock(m_mutex);
    m_audio.reset(new RtAudio(RtAudio::WINDOWS_ASIO, errorCallback));
    if (m_audio->getCurrentApi() != RtAudio::WINDOWS_ASIO) {
        if (error) {
            *error = "RtAudio ASIO API is not available in this build.";
        }
        m_audio.reset();
        return false;
    }

    const RtAudio::DeviceInfo info = m_audio->getDeviceInfo(deviceId);
    if (info.outputChannels == 0) {
        if (error) {
            *error = "Selected device has no output channels";
        }
        m_audio.reset();
        return false;
    }
    if (channelOffset >= info.outputChannels) {
        if (error) {
            *error = "Channel offset exceeds device output channels";
        }
        m_audio.reset();
        return false;
    }

    const unsigned int outChannels = std::min(
        std::max(1u, m_wav.channels()),
        info.outputChannels - channelOffset);

    static PlaybackState state;
    state.samples = m_wav.samples();
    state.totalFrames = m_wav.frameCount();
    state.readFrame = 0;
    state.wavChannels = m_wav.channels();
    state.outChannels = outChannels;
    state.channelOffset = channelOffset;
    state.loop = m_loop;
    state.stopRequested = &m_stopRequested;

    RtAudio::StreamParameters params;
    params.deviceId = deviceId;
    params.nChannels = outChannels;
    params.firstChannel = channelOffset;

    RtAudio::StreamOptions options;
    options.flags = RTAUDIO_MINIMIZE_LATENCY;

    unsigned int bufferFrames = 512;
    const unsigned int sampleRate = m_wav.sampleRate();

    const RtAudioErrorType openResult = m_audio->openStream(&params,
                                                            nullptr,
                                                            RTAUDIO_SINT16,
                                                            sampleRate,
                                                            &bufferFrames,
                                                            &playbackCallback,
                                                            &state,
                                                            &options);
    if (openResult != RTAUDIO_NO_ERROR) {
        if (error) {
            *error = "openStream failed, error code " + std::to_string(openResult);
        }
        m_audio.reset();
        return false;
    }

    const RtAudioErrorType startResult = m_audio->startStream();
    if (startResult != RTAUDIO_NO_ERROR) {
        if (error) {
            *error = "startStream failed, error code " + std::to_string(startResult);
        }
        m_audio->closeStream();
        m_audio.reset();
        return false;
    }

    log("Playing on device " + info.name + " @ " + std::to_string(sampleRate) + " Hz, "
        + std::to_string(outChannels) + " ch, buffer=" + std::to_string(bufferFrames));
    return true;
}

void LtcWavPlayer::requestStop() {
    m_stopRequested.store(true);
}

void LtcWavPlayer::stopBlocking() {
    m_stopRequested.store(true);

    std::unique_ptr<RtAudio> audio;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        audio = std::move(m_audio);
    }

    if (!audio) {
        m_stopRequested.store(false);
        return;
    }

    if (audio->isStreamRunning()) {
        audio->abortStream();
    }
    if (audio->isStreamOpen()) {
        audio->closeStream();
    }

    m_stopRequested.store(false);
}

bool LtcWavPlayer::isPlaying() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_audio && m_audio->isStreamRunning();
}

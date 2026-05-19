#ifndef WAVREADER_H
#define WAVREADER_H

#include <cstdint>
#include <string>
#include <vector>

class WavReader {
public:
    bool load(const std::string& path, std::string* error = nullptr);

    unsigned int sampleRate() const { return m_sampleRate; }
    unsigned int channels() const { return m_channels; }
    unsigned int bitsPerSample() const { return m_bitsPerSample; }
    unsigned int frameCount() const { return m_frameCount; }
    const int16_t* samples() const { return m_samples.data(); }
    const std::vector<int16_t>& sampleData() const { return m_samples; }

private:
    unsigned int m_sampleRate = 0;
    unsigned int m_channels = 0;
    unsigned int m_bitsPerSample = 0;
    unsigned int m_frameCount = 0;
    std::vector<int16_t> m_samples;
};

#endif

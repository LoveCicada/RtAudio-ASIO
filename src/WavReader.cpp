#include "WavReader.h"

#include <cstring>
#include <fstream>

namespace {

bool readExact(std::ifstream& in, void* dst, std::size_t size) {
    in.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(size));
    return static_cast<std::size_t>(in.gcount()) == size;
}

uint16_t readU16(const unsigned char* p) {
    return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
}

uint32_t readU32(const unsigned char* p) {
    return static_cast<uint32_t>(p[0])
        | (static_cast<uint32_t>(p[1]) << 8)
        | (static_cast<uint32_t>(p[2]) << 16)
        | (static_cast<uint32_t>(p[3]) << 24);
}

} // namespace

bool WavReader::load(const std::string& path, std::string* error) {
    m_sampleRate = 0;
    m_channels = 0;
    m_bitsPerSample = 0;
    m_frameCount = 0;
    m_samples.clear();

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        if (error) {
            *error = "Cannot open WAV file: " + path;
        }
        return false;
    }

    unsigned char riff[12] = {0};
    if (!readExact(in, riff, 12)) {
        if (error) {
            *error = "Invalid WAV header";
        }
        return false;
    }
    if (std::memcmp(riff, "RIFF", 4) != 0 || std::memcmp(riff + 8, "WAVE", 4) != 0) {
        if (error) {
            *error = "Not a RIFF/WAVE file";
        }
        return false;
    }

    bool haveFmt = false;
    bool haveData = false;
    uint16_t audioFormat = 0;
    uint32_t dataSize = 0;

    while (in && !(haveFmt && haveData)) {
        unsigned char chunkHeader[8] = {0};
        if (!readExact(in, chunkHeader, 8)) {
            break;
        }
        const uint32_t chunkSize = readU32(chunkHeader + 4);

        if (std::memcmp(chunkHeader, "fmt ", 4) == 0) {
            if (chunkSize < 16) {
                if (error) {
                    *error = "Invalid fmt chunk";
                }
                return false;
            }
            unsigned char fmt[16] = {0};
            if (!readExact(in, fmt, 16)) {
                if (error) {
                    *error = "Truncated fmt chunk";
                }
                return false;
            }
            audioFormat = readU16(fmt);
            m_channels = readU16(fmt + 2);
            m_sampleRate = readU32(fmt + 4);
            m_bitsPerSample = readU16(fmt + 14);

            if (chunkSize > 16) {
                in.seekg(static_cast<std::streamoff>(chunkSize - 16), std::ios::cur);
            }
            haveFmt = true;
        } else if (std::memcmp(chunkHeader, "data", 4) == 0) {
            dataSize = chunkSize;
            haveData = true;
            break;
        } else {
            in.seekg(static_cast<std::streamoff>(chunkSize), std::ios::cur);
        }
    }

    if (!haveFmt || !haveData) {
        if (error) {
            *error = "Missing fmt or data chunk";
        }
        return false;
    }
    if (audioFormat != 1) {
        if (error) {
            *error = "Only PCM WAV is supported";
        }
        return false;
    }
    if (m_bitsPerSample != 16) {
        if (error) {
            *error = "Only 16-bit PCM WAV is supported";
        }
        return false;
    }
    if (m_channels < 1 || m_channels > 2) {
        if (error) {
            *error = "Only mono or stereo WAV is supported";
        }
        return false;
    }

    const std::size_t bytesPerFrame = static_cast<std::size_t>(m_channels) * 2;
    if (dataSize % bytesPerFrame != 0) {
        if (error) {
            *error = "WAV data size is not frame-aligned";
        }
        return false;
    }

    m_frameCount = static_cast<unsigned int>(dataSize / bytesPerFrame);
    m_samples.resize(m_frameCount * m_channels);

    if (!readExact(in, m_samples.data(), dataSize)) {
        if (error) {
            *error = "Failed to read WAV PCM data";
        }
        m_samples.clear();
        return false;
    }

    return true;
}

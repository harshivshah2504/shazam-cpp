#ifndef WAV_H
#define WAV_H

#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sstream>

struct WavHeader {
    char ChunkID[4];
    uint32_t ChunkSize;
    char Format[4];
    char Subchunk1ID[4];
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t BytesPerSec;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    char Subchunk2ID[4];
    uint32_t Subchunk2Size;
};

class WAVProcessor {
public:
    static void WriteWavFile(const std::string& filename, const std::vector<uint8_t>& data, int sampleRate, int channels, int bitsPerSample);
    static std::vector<uint8_t> ReadWavFile(const std::string& filename);
    static std::vector<float> WavBytesToSamples(const std::vector<uint8_t>& input);
    static std::string GetMetadata(const std::string& filePath);
};

#endif // WAV_H

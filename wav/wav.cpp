#include "wav.h"
#include <filesystem>
#include <sstream>
#include <cstdlib>
#include <cstring> 


// Function to write WAV file
void WAVProcessor::WriteWavFile(const std::string& filename, const std::vector<uint8_t>& data, int sampleRate, int channels, int bitsPerSample) {
    if (sampleRate <= 0 || channels <= 0 || bitsPerSample <= 0) {
        throw std::invalid_argument("Invalid parameters: sampleRate, channels, and bitsPerSample must be positive.");
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Prepare WAV header
    WavHeader header;
    std::memcpy(header.ChunkID, "RIFF", 4);
    header.ChunkSize = 36 + data.size();
    std::memcpy(header.Format, "WAVE", 4);
    std::memcpy(header.Subchunk1ID, "fmt ", 4);
    header.Subchunk1Size = 16;
    header.AudioFormat = 1; // PCM format
    header.NumChannels = channels;
    header.SampleRate = sampleRate;
    int bytesPerSample = bitsPerSample / 8;
    header.BytesPerSec = sampleRate * channels * bytesPerSample;
    header.BlockAlign = channels * bytesPerSample;
    header.BitsPerSample = bitsPerSample;
    std::memcpy(header.Subchunk2ID, "data", 4);
    header.Subchunk2Size = data.size();

    // Write header and data
    file.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
}

// Function to read a WAV file
std::vector<uint8_t> WAVProcessor::ReadWavFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    // Read header
    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

    // Validate WAV format
    if (std::string(header.ChunkID, 4) != "RIFF" || std::string(header.Format, 4) != "WAVE") {
        throw std::runtime_error("Invalid WAV file format.");
    }

    // Read audio data
    std::vector<uint8_t> data(header.Subchunk2Size);
    file.read(reinterpret_cast<char*>(data.data()), header.Subchunk2Size);
    file.close();

    return data;
}

// Function to convert WAV bytes to float samples
std::vector<float> WAVProcessor::WavBytesToSamples(const std::vector<uint8_t>& input) {
    if (input.size() % 2 != 0) {
        throw std::runtime_error("Invalid input size (not divisible by 2)");
    }

    std::vector<float> output;
    for (size_t i = 0; i < input.size(); i += 2) {
        int16_t sample = static_cast<int16_t>(input[i] | (input[i + 1] << 8));
        output.push_back(sample / 32768.0f); // Normalize to [-1, 1]
    }

    return output;
}

// Function to get metadata using ffprobe
std::string WAVProcessor::GetMetadata(const std::string& filePath) {
    std::string command = "ffprobe -v quiet -print_format json -show_format -show_streams \"" + filePath + "\"";
    std::ostringstream output;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute ffprobe command.");
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output << buffer;
    }

    pclose(pipe);
    return output.str();
}

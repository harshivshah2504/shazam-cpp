#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <cstdlib>
#include <iomanip>

// Get an environment variable with a fallback value
std::string getEnv(const std::string& key, const std::string& defaultValue) {
    const char* value = std::getenv(key.c_str());
    return value ? std::string(value) : defaultValue;
}

// Function to delete a file
bool DeleteFile(const std::string& filePath) {
    return fs::exists(filePath) && fs::remove(filePath);
}

// Function to create a folder
bool CreateFolder(const std::string& folderPath) {
    return fs::create_directories(folderPath);
}

// Convert float samples to bytes based on bit depth
std::vector<uint8_t> FloatsToBytes(const std::vector<float>& data, int bitsPerSample) {
    std::vector<uint8_t> byteData;
    switch (bitsPerSample) {
        case 8:
            for (float sample : data)
                byteData.push_back(static_cast<uint8_t>((sample + 1.0) * 127.5));
            break;
        case 16:
            for (float sample : data) {
                int16_t val = static_cast<int16_t>(sample * 32767.0);
                byteData.push_back(val & 0xFF);
                byteData.push_back((val >> 8) & 0xFF);
            }
            break;
        case 24:
            for (float sample : data) {
                int32_t val = static_cast<int32_t>(sample * 8388607.0);
                byteData.push_back((val >> 8) & 0xFF);
                byteData.push_back((val >> 16) & 0xFF);
                byteData.push_back((val >> 24) & 0xFF);
            }
            break;
        case 32:
            for (float sample : data) {
                int32_t val = static_cast<int32_t>(sample * 2147483647.0);
                byteData.push_back(val & 0xFF);
                byteData.push_back((val >> 8) & 0xFF);
                byteData.push_back((val >> 16) & 0xFF);
                byteData.push_back((val >> 24) & 0xFF);
            }
            break;
        default:
            throw std::runtime_error("Unsupported bits per sample");
    }
    return byteData;
}

// Function to generate a unique ID
uint32_t GenerateUniqueID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis;
    return dis(gen);
}

// Generate a unique song key based on title and artist
std::string GenerateSongKey(const std::string& songTitle, const std::string& songArtist) {
    return songTitle + "---" + songArtist;
}

// Function to get the current timestamp for file naming
std::string GetTimestamp() {
    auto now = std::time(nullptr);
    struct tm* timeinfo = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%Y_%m_%d_%H_%M_%S");
    return oss.str();
}

// Process a recorded audio file
std::vector<float> ProcessRecording(const std::vector<uint8_t>& audioData, int sampleRate, int channels, int sampleSize, bool saveRecording) {
    std::string fileName = GetTimestamp() + ".wav";
    std::string filePath = "tmp/" + fileName;

    // Ensure tmp directory exists
    CreateFolder("tmp");

    // Save the raw audio data
    std::ofstream outFile(filePath, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(audioData.data()), audioData.size());
    outFile.close();

    // Simulate reading the WAV file back
    std::vector<float> samples;
    for (size_t i = 0; i < audioData.size(); i += sampleSize / 8) {
        float sample = static_cast<float>(audioData[i]) / 255.0f; // Fake conversion for demo
        samples.push_back(sample);
    }

    // If saveRecording is true, move file to "recordings" folder
    if (saveRecording) {
        CreateFolder("recordings");
        std::string newFilePath = "recordings/" + fileName;
        fs::rename(filePath, newFilePath);
    }

    // Delete temporary file
    DeleteFile(filePath);

    return samples;
}

#include <header/utils.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <cstdlib>
#include <iomanip>


std::string getEnv(const std::string& key, const std::string& defaultValue) {
    const char* value = std::getenv(key.c_str());
    return value ? std::string(value) : defaultValue;
}


bool DeleteFile(const std::string& filePath) {
    return fs::exists(filePath) && fs::remove(filePath);
}


bool CreateFolder(const std::string& folderPath) {
    return fs::create_directories(folderPath);
}


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


uint32_t GenerateUniqueID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis;
    return dis(gen);
}


std::string GenerateSongKey(const std::string& songTitle, const std::string& songArtist) {
    return songTitle + "---" + songArtist;
}


std::string GetTimestamp() {
    auto now = std::time(nullptr);
    struct tm* timeinfo = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%Y_%m_%d_%H_%M_%S");
    return oss.str();
}


std::vector<float> ProcessRecording(const std::vector<uint8_t>& audioData, int sampleRate, int channels, int sampleSize, bool saveRecording) {
    std::string fileName = GetTimestamp() + ".wav";
    std::string filePath = "tmp/" + fileName;


    CreateFolder("tmp");


    std::ofstream outFile(filePath, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(audioData.data()), audioData.size());
    outFile.close();


    std::vector<float> samples;
    for (size_t i = 0; i < audioData.size(); i += sampleSize / 8) {
        float sample = static_cast<float>(audioData[i]) / 255.0f; 
        samples.push_back(sample);
    }


    if (saveRecording) {
        CreateFolder("recordings");
        std::string newFilePath = "recordings/" + fileName;
        fs::rename(filePath, newFilePath);
    }


    DeleteFile(filePath);

    return samples;
}

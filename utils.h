#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;


std::string getEnv(const std::string& key, const std::string& defaultValue = "");
bool DeleteFile(const std::string& filePath);
bool CreateFolder(const std::string& folderPath);
std::vector<uint8_t> FloatsToBytes(const std::vector<float>& data, int bitsPerSample);
uint32_t GenerateUniqueID();
std::string GenerateSongKey(const std::string& songTitle, const std::string& songArtist);
std::string GetEnv(const std::string& key, const std::string& fallback = "");
std::string GetTimestamp();
std::vector<float> ProcessRecording(const std::vector<uint8_t>& audioData, int sampleRate, int channels, int sampleSize, bool saveRecording);

#endif  

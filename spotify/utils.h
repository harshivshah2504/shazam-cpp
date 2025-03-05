#ifndef SPOTIFY_UTILS_H
#define SPOTIFY_UTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <sys/stat.h>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <regex>

// URL Encode Function
std::string EncodeParam(const std::string &s) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::uppercase << std::setw(2) << int((unsigned char)c);
        }
    }
    return escaped.str();
}

// Convert String to Lowercase
std::string ToLowerCase(const std::string &s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

// Get File Size
int64_t GetFileSize(const std::string &filePath) {
    struct stat stat_buf;
    if (stat(filePath.c_str(), &stat_buf) == 0) {
        return stat_buf.st_size;
    }
    return -1; // Error
}

// Check if a File Exists
bool FileExists(const std::string &filePath) {
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

// Validate Song Key Existence (Dummy Implementation)
bool SongKeyExists(const std::string &key) {
    // In actual implementation, connect to DB and check key existence
    return false; 
}

// Validate YouTube ID Existence (Dummy Implementation)
bool YtIDExists(const std::string &ytID) {
    // In actual implementation, connect to DB and check ID existence
    return false;
}

// Correct Invalid File Names for Windows
std::pair<std::string, std::string> CorrectFilename(std::string title, std::string artist) {
    std::vector<char> invalidChars = {'<', '>', ':', '"', '\\', '/', '|', '?', '*'};

#ifdef _WIN32
    for (char invalid : invalidChars) {
        title.erase(std::remove(title.begin(), title.end(), invalid), title.end());
        artist.erase(std::remove(artist.begin(), artist.end(), invalid), artist.end());
    }
#else
    std::replace(title.begin(), title.end(), '/', '\\');
    std::replace(artist.begin(), artist.end(), '/', '\\');
#endif

    return {title, artist};
}

// Convert Stereo to Mono Using FFmpeg
std::vector<uint8_t> ConvertStereoToMono(const std::string &stereoFilePath) {
    std::string fileExt = stereoFilePath.substr(stereoFilePath.find_last_of("."));
    std::string monoFilePath = stereoFilePath.substr(0, stereoFilePath.find_last_of(".")) + "_mono" + fileExt;

    // Check the number of channels using FFprobe
    std::string command = "ffprobe -v error -show_entries stream=channels -of default=noprint_wrappers=1:nokey=1 \"" + stereoFilePath + "\"";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) throw std::runtime_error("Failed to run ffprobe");

    char buffer[128];
    std::string channels;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        channels += buffer;
    }
    pclose(pipe);

    if (channels.find("1") == std::string::npos) {  // If not mono, convert
        std::string convertCmd = "ffmpeg -i \"" + stereoFilePath + "\" -af \"pan=mono|c0=c0\" \"" + monoFilePath + "\"";
        system(convertCmd.c_str());
    }

    // Read file into byte array
    std::ifstream file(monoFilePath, std::ios::binary);
    std::vector<uint8_t> audioData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Remove temporary mono file
    remove(monoFilePath.c_str());

    return audioData;
}

#endif // SPOTIFY_UTILS_H

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <sstream>
#include <thread>
#include <mutex>
#include <fstream>
#include <random>
#include <future>
#include <nlohmann/json.hpp>
#include "youtube.h"
#include <mongo.h>
#include "shazam.h"
#include "wav.h"
#include "utils.h"
#include <youtube.h>
#include <spectogram.h>
#include <fingerprint.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

const bool DELETE_SONG_FILE = false;
const std::string SONGS_DIR = "songs";

// Logger utility
void logMessage(const std::string& message) {
    std::cout << "[LOG] " << message << std::endl;
}

// Function to create a directory
bool createFolder(const std::string& path) {
    return fs::create_directories(path);
}

// Function to delete a file
bool deleteFile(const std::string& path) {
    return fs::remove(path);
}

// Function to generate a random unique ID
uint32_t generateUniqueID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis;
    return dis(gen);
}

// Function to download a single track
int DlSingleTrack(const std::string& url, const std::string& savePath) {
    std::cout << "Getting track info...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::string trackInfo = getYouTubeTrackInfo(url);
    if (trackInfo.empty()) {
        return 0;
    }

    std::cout << "Now downloading track...\n";
    return downloadTrack(trackInfo, savePath) ? 1 : 0;
}

// Function to download a playlist
int DlPlaylist(const std::string& url, const std::string& savePath) {
    std::vector<std::string> tracks = getYouTubePlaylistInfo(url);
    if (tracks.empty()) return 0;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Now downloading playlist...\n";

    int totalTracksDownloaded = 0;
    for (const auto& track : tracks) {
        if (downloadTrack(track, savePath)) {
            totalTracksDownloaded++;
        }
    }
    return totalTracksDownloaded;
}

// Function to download an album
int DlAlbum(const std::string& url, const std::string& savePath) {
    std::vector<std::string> tracks = getYouTubeAlbumInfo(url);
    if (tracks.empty()) return 0;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Now downloading album...\n";

    int totalTracksDownloaded = 0;
    for (const auto& track : tracks) {
        if (downloadTrack(track, savePath)) {
            totalTracksDownloaded++;
        }
    }
    return totalTracksDownloaded;
}

// Function to process and store a song
bool processAndSaveSong(const std::string& songFilePath, const std::string& songTitle, const std::string& songArtist, const std::string& ytID) {
    logMessage("Processing song: " + songTitle + " by " + songArtist);

    std::unique_ptr<DBClient> dbclient = NewDBClient();
    if (!dbclient) {
        logMessage("Error initializing database client.");
        return false;
    }

    std::string wavFilePath = convertToWAV(songFilePath, 1);
    if (wavFilePath.empty()) {
        logMessage("Failed to convert to WAV.");
        return false;
    }

    auto wavInfo = readWavInfo(wavFilePath);
    std::vector<double> samples = wavBytesToSamples(wavInfo.data);

    uint32_t songID = dbclient->RegisterSong(songTitle, songArtist, ytID);

    auto spectrogram = Spectrogram(samples, wavInfo.sampleRate);
    auto peaks = ExtractPeaks(spectrogram, wavInfo.duration);
    auto fingerprints = Fingerprint(peaks, songID);

    if (!dbclient->StoreFingerprints(fingerprints)) {
        dbclient->DeleteSongByID(songID);
        logMessage("Error storing fingerprints.");
        return false;
    }

    logMessage("Fingerprint for " + songTitle + " by " + songArtist + " saved in DB successfully.");
    return true;
}

// Function to download YouTube audio
bool downloadYTaudio(const std::string& id, const std::string& savePath, std::string& filePath) {
    std::string outputFile = savePath + "/" + id + ".m4a";
    
    std::string command = "yt-dlp -f 140 -o \"" + outputFile + "\" \"https://www.youtube.com/watch?v=" + id + "\"";
    if (std::system(command.c_str()) != 0) {
        logMessage("Failed to download YouTube audio.");
        return false;
    }

    filePath = outputFile;
    return true;
}

// Function to add metadata tags using FFmpeg
bool addTags(const std::string& file, const std::string& title, const std::string& artist, const std::string& album) {
    std::string tempFile = file + "_temp.wav";
    
    std::string command = "ffmpeg -i \"" + file + "\" -c copy "
                          "-metadata title=\"" + title + "\" "
                          "-metadata artist=\"" + artist + "\" "
                          "-metadata album=\"" + album + "\" "
                          "\"" + tempFile + "\"";

    if (std::system(command.c_str()) != 0) {
        logMessage("Failed to add tags.");
        return false;
    }

    fs::rename(tempFile, file);
    return true;
}

// Function to get YouTube ID for a track
std::string getYTID(const std::string& trackTitle, const std::string& trackArtist) {
    std::string ytID = searchYouTube(trackTitle + " " + trackArtist);
    if (ytID.empty()) {
        logMessage("Failed to get YouTube ID.");
        return "";
    }

    return ytID;
}

// Function to erase the database and delete song files
void eraseDatabase() {
    logMessage("Erasing database and song files...");

    std::unique_ptr<DBClient> dbClient = NewDBClient();
    if (!dbClient) {
        logMessage("Error initializing database client.");
        return;
    }

    dbClient->DeleteCollection("fingerprints");
    dbClient->DeleteCollection("songs");

    for (const auto& entry : fs::directory_iterator(SONGS_DIR)) {
        if (entry.path().extension() == ".wav" || entry.path().extension() == ".m4a") {
            fs::remove(entry.path());
        }
    }

    logMessage("Erase complete!");
}

// Main function
int main() {
    logMessage("Starting song processing...");

    std::string url = "https://www.youtube.com/watch?v=exampleID";
    std::string savePath = "downloads";

    createFolder(savePath);
    int totalDownloaded = DlSingleTrack(url, savePath);
    
    logMessage("Total tracks downloaded: " + std::to_string(totalDownloaded));
    return 0;
}

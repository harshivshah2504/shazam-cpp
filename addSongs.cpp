#include <audiorw.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <header/fingerprint.h>
#include <header/spectogram.h>
#include <header/mongo.h>
#include <header/mp3.h>



bool ProcessAndSaveSong(const std::string& songFilePath, const std::string& songTitle, const std::string& songArtist) {
    try {
        std::unique_ptr<DBClient> db = NewDBClient(); 
        if (!db->Connect()) {
            throw std::runtime_error("Database connection failed.");
        }

        auto [samples, sampleRate, channels, duration] = decodeMP3ToFloat(songFilePath);
        if (samples.empty()) {
            throw std::runtime_error("Error converting MP3 bytes to samples.");
        }

        auto spectrogram = Spectrogram(samples, sampleRate); 
        if (spectrogram.empty()) {
            throw std::runtime_error("Error creating spectrogram.");
        }

        uint32_t songID = db->RegisterSong(songTitle, songArtist);
        std::vector<Peak> peaks = ExtractPeaks(spectrogram, duration); 
        if (peaks.empty()) {
            throw std::runtime_error("No peaks found in spectrogram.");
        }

        auto fingerprints = Fingerprint(peaks, songID); 
        if (fingerprints.empty()) {
            throw std::runtime_error("Failed to generate fingerprints.");
        }

        bool success = db->StoreFingerprints(fingerprints);
        if (!success) {
            db->DeleteSongByID(songID);  
            throw std::runtime_error("Failed to store fingerprints in database.");
        }

        return true;
    } 
    catch (const std::exception& e) {
        std::cerr << "Error processing song: " << e.what() << std::endl;
        return false;
    }
}


int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: ./add <songFilePath> <songTitle> <songArtist>" << std::endl;
        return 1;
    }

    std::string songFilePath = argv[1];  
    std::string songTitle = argv[2];    
    std::string songArtist = argv[3];   
    bool done = ProcessAndSaveSong(songFilePath, songTitle, songArtist);

    return 0;
}
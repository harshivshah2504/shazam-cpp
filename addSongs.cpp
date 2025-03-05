#include <audiorw.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <fingerprint.h>
#include <spectogram.h>
#include <mongo.h>
#include <mp3.h>



// Function to process and save the song in the database
bool ProcessAndSaveSong(const std::string& songFilePath, const std::string& songTitle, const std::string& songArtist, const std::string& ytID) {
    try {
        // Step 1: Initialize the database client
        std::unique_ptr<DBClient> db = NewDBClient(); 
        if (!db->Connect()) {
            throw std::runtime_error("Database connection failed.");
        }

        // Step 3: Convert WAV bytes to float64 samples
        auto [samples, sampleRate, channels, duration] = decodeMP3ToFloat(songFilePath);
        if (samples.empty()) {
            throw std::runtime_error("Error converting MP3 bytes to samples.");
        }

        // Step 4: Create spectrogram from audio samples
        auto spectrogram = Spectrogram(samples, sampleRate); // Implement spectrogram function
        if (spectrogram.empty()) {
            throw std::runtime_error("Error creating spectrogram.");
        }

        // Step 5: Register song in database and get song ID
        uint32_t songID = db->RegisterSong(songTitle, songArtist, ytID);

        // Step 6: Extract peaks from spectrogram
        std::vector<Peak> peaks = ExtractPeaks(spectrogram, duration); // Implement peak extraction
        if (peaks.empty()) {
            throw std::runtime_error("No peaks found in spectrogram.");
        }

        auto fingerprints = Fingerprint(peaks, songID); 
        if (fingerprints.empty()) {
            throw std::runtime_error("Failed to generate fingerprints.");
        }

        // Step 8: Store fingerprints in the database
        bool success = db->StoreFingerprints(fingerprints);
        if (!success) {
            db->DeleteSongByID(songID);  
            throw std::runtime_error("Failed to store fingerprints in database.");
        }

        // Print success message
        std::cout << "Fingerprint for " << songTitle << " by " << songArtist << " saved in DB successfully!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error processing song: " << e.what() << std::endl;
        return false;
    }
}


int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: ./add <songFilePath> <songTitle> <songArtist> <YouTubeID>" << std::endl;
        return 1;
    }

    // Get the command-line arguments
    std::string songFilePath = argv[1];  // Path to the WAV file
    std::string songTitle = argv[2];     // Song title
    std::string songArtist = argv[3];    // Song artist
    std::string ytID = argv[4];          // YouTube ID

    // Call the function to process and save the song
    if (ProcessAndSaveSong(songFilePath, songTitle, songArtist, ytID)) {
        std::cout << "Song processed and saved successfully." << std::endl;
    } else {
        std::cout << "Failed to process and save the song." << std::endl;
    }

    return 0;
}
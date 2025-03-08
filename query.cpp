#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <mongo.h>
#include <utils.h>
#include <spectogram.h>
#include <fingerprint.h>
#include <mp3.h>

// Structure to store match results
struct Match {
    uint32_t songID;
    std::string songTitle;
    std::string songArtist;
    uint32_t timestamp;
    double score;

    Match(uint32_t id, const std::string& title, const std::string& artist,
          uint32_t time, double sc)
        : songID(id), songTitle(title), songArtist(artist),
          timestamp(time), score(sc) {}
};

// Function to analyze timing differences for fingerprint matching
std::map<uint32_t, double> analyzeRelativeTiming(
    const std::map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>& matches
) {
    std::map<uint32_t, double> scores;
    for (const auto& [songID, times] : matches) {
        int count = 0;
        for (size_t i = 0; i < times.size(); ++i) {
            for (size_t j = i + 1; j < times.size(); ++j) {
                double sampleDiff = std::abs(static_cast<double>(times[i].first) - times[j].first);
                double dbDiff = std::abs(static_cast<double>(times[i].second) - times[j].second);
                if (std::abs(sampleDiff - dbDiff) < 100) {
                    count++;
                }
            }
        }
        scores[songID] = static_cast<double>(count);
    }
    return scores;
}

// Function to find a match for a given audio sample
std::vector<Match> FindMatch(const std::vector<double>& audioSamples, long audioDuration, double sampleRate) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // Step 1: Generate spectrogram
    auto spectrogram = Spectrogram(audioSamples, sampleRate);
    if (spectrogram.empty()) {
        throw std::runtime_error("Failed to generate spectrogram.");
    }

    // Step 2: Extract peaks and generate fingerprints
    auto peaks = ExtractPeaks(spectrogram, audioDuration);
    auto fingerprints = Fingerprint(peaks, GenerateUniqueID());

    std::vector<uint32_t> addresses;
    for (const auto& fp : fingerprints) {
        addresses.push_back(fp.first);
    }

    // Step 3: Connect to MongoDB Atlas (or fallback to local)
    MongoClient db("mongodb://localhost:27017");
    if (!db.Connect()) {
        throw std::runtime_error("Database connection failed.");
    }

    // Step 4: Fetch matching fingerprint data
    auto matchesData = db.GetCouples(addresses);
    std::map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> matches;
    std::map<uint32_t, std::vector<uint32_t>> timestamps;

    for (const auto& [address, couples] : matchesData) {
        for (const auto& couple : couples) {
            matches[couple.songID].emplace_back(fingerprints[address].anchorTimeMs, couple.anchorTimeMs);
            timestamps[couple.songID].push_back(couple.anchorTimeMs);
        }
    }

    // Step 5: Analyze relative timing
    auto scores = analyzeRelativeTiming(matches);
    std::vector<Match> matchList;

    // Step 6: Retrieve song details from DB
    for (const auto& [songID, points] : scores) {
        auto song = db.GetSongByID(songID);
        if (!song) continue;

        std::sort(timestamps[songID].begin(), timestamps[songID].end());
        Match match(songID, song->title, song->artist, timestamps[songID][0], points);
        matchList.push_back(match);
    }

    // Step 7: Sort matches by score
    std::sort(matchList.begin(), matchList.end(), [](const Match& a, const Match& b) {
        return a.score > b.score;
    });

    return matchList;
}

// Function to process and find a song match
void findSongMatch(const std::string& filePath) {
    try {
        // Step 1: Decode the MP3 file and extract float64 samples
        auto [samples, sampleRate, channels, duration] = decodeMP3ToFloat(filePath);
        if (samples.empty()) {
            throw std::runtime_error("Error converting MP3 bytes to samples.");
        }

        // Step 2: Find matches using extracted samples
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<Match> matches = FindMatch(samples, duration, sampleRate);
        auto end = std::chrono::high_resolution_clock::now();

        // Step 3: Calculate search duration
        std::chrono::duration<double> searchDuration = end - start;

        // Step 4: Display match results
        if (matches.empty()) {
            std::cout << "\nNo match found." << std::endl;
        } else {
            std::cout << "\nMatches Found:\n";
            for (size_t i = 0; i < std::min(matches.size(), static_cast<size_t>(20)); ++i) {
                std::cout << matches[i].songTitle << " by " << matches[i].songArtist
                          << ", score: " << std::fixed << std::setprecision(2) << matches[i].score << std::endl;
            }

            // Display top match prediction
            std::cout << "\nBest Match: " << matches[0].songTitle << " by " << matches[0].songArtist
                      << ", score: " << std::fixed << std::setprecision(2) << matches[0].score << std::endl;
        }

        // Step 5: Display search duration
        std::cout << "\nSearch took: " << searchDuration.count() << " seconds" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// Main function
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./shazam <audio_file_path>" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    findSongMatch(filePath);

    return 0;
}

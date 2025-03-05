#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <mongo.h>  
#include <utils.h>
#include <fingerprint.h>
#include <spectogram.h>

// Struct to store match results
struct Match {
    uint32_t songID;
    std::string songTitle;
    std::string songArtist;
    std::string youTubeID;
    uint32_t timestamp;
    double score;

    Match(uint32_t songID, const std::string& songTitle, const std::string& songArtist,
          const std::string& youTubeID, uint32_t timestamp, double score)
        : songID(songID), songTitle(songTitle), songArtist(songArtist), 
          youTubeID(youTubeID), timestamp(timestamp), score(score) {}
};

// Function prototypes
std::map<uint32_t, double> analyzeRelativeTiming(
    const std::map<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>& matches
);

// Function to find a match for a given audio sample
std::vector<Match> FindMatch(const std::vector<double>& audioSamples, long audioDuration, double sampleRate) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // Step 1: Generate spectrogram
    auto spectrogram = Spectrogram(audioSamples, sampleRate);
    if (spectrogram.empty()) {
        throw std::runtime_error("Failed to get spectrogram of samples");
    }

    // Step 2: Extract peaks and generate fingerprints
    auto peaks = ExtractPeaks(spectrogram, audioDuration);
    auto fingerprints = Fingerprint(peaks, GenerateUniqueID());

    std::vector<uint32_t> addresses;
    for (const auto& fp : fingerprints) {
        addresses.push_back(fp.first);
    }

    // Step 3: Connect to MongoDB
    MongoClient db("mongodb://localhost:27017");  
    if (!db.Connect()) {
        throw std::runtime_error("Database connection failed");
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

        Match match(songID, song->title, song->artist, song->youTubeID, timestamps[songID][0], points);
        matchList.push_back(match);
    }

    // Step 7: Sort matches by score
    std::sort(matchList.begin(), matchList.end(), [](const Match& a, const Match& b) {
        return a.score > b.score;
    });

    return matchList;
}

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
                if (std::abs(sampleDiff - dbDiff) < 50) {
                    count++;
                }
            }
        }
        scores[songID] = static_cast<double>(count);
    }
    return scores;
}

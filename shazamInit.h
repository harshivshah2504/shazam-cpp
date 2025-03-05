#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "mongo.h"      // Assume this is the database client
#include "./models.h" 
#include <spectogram.h> 
#include <fingerprint.h>
#include "utils.h"   // Assume this contains utility functions

struct Match {
    uint32_t songID;
    std::string songTitle;
    std::string songArtist;
    std::string youTubeID;
    uint32_t timestamp;
    double coherency;
};



std::vector<Match> Search(std::vector<float>& audioSamples, double audioDuration, int sampleRate) {
    auto spectrogram = Spectrogram(audioSamples, sampleRate);
    auto peaks = ExtractPeaks(spectrogram, audioDuration);
    auto fingerprints = Fingerprint(peaks, GenerateUniqueID());

    std::vector<uint32_t> addresses;
    for (const auto& [address, _] : fingerprints) {
        addresses.push_back(address);
    }

    MongoClient db("mongodb://localhost:27017");  
    if (!db.Connect()) {
        throw std::runtime_error("Failed to connect to database");
    }

    auto couples = db.GetCouples(addresses);
    auto targetZones = ComputeTargetZones(couples);
    std::cout << "TargetZones: " << targetZones.size() << std::endl;


    auto matches = ComputeTimeCoherency(fingerprints, targetZones);
    std::vector<Match> matchList;

    for (const auto& [songID, coherency] : matches) {
        auto song = db.GetSongByID(songID);
        if (!song) continue;

        uint32_t timestamp = targetZones[songID][0];
        matchList.push_back({songID, song->title, song->artist, song->youTubeID, timestamp, static_cast<double>(coherency)});
    }

    std::sort(matchList.begin(), matchList.end(), [](const Match& a, const Match& b) {
        return a.coherency > b.coherency;
    });

    return matchList;
}

std::map<uint32_t, std::vector<uint32_t>> ComputeTargetZones(const std::map<uint32_t, std::vector<Couple>>& couples) {
    std::map<uint32_t, std::map<uint32_t, int>> songAnchorCounts;

    for (const auto& [_, songCouples] : couples) {
        for (const auto& couple : songCouples) {
            songAnchorCounts[couple.songID][couple.anchorTimeMs]++;
        }
    }

    for (auto& [songID, anchorCounts] : songAnchorCounts) {
        for (auto it = anchorCounts.begin(); it != anchorCounts.end(); ) {
            if (it->second < 5) {
                it = anchorCounts.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::map<uint32_t, std::vector<uint32_t>> targetZones;
    for (const auto& [songID, anchors] : songAnchorCounts) {
        for (const auto& [anchorTime, _] : anchors) {
            targetZones[songID].push_back(anchorTime);
        }
    }

    return targetZones;
}

std::map<uint32_t, int> ComputeTimeCoherency(const std::unordered_map<uint32_t, Couple>& record, const std::map<uint32_t, std::vector<uint32_t>>& targetZones) {
    std::map<uint32_t, int> matches;

    for (const auto& [songID, anchorTimes] : targetZones) {
        std::map<double, int> deltas;
        for (uint32_t songAnchorTime : anchorTimes) {
            for (const auto& [_, recordAnchor] : record) {
                double delta = static_cast<double>(recordAnchor.anchorTimeMs) - songAnchorTime;
                deltas[delta]++;
            }
        }

        int maxOccurrences = 0;
        for (const auto& [_, occurrences] : deltas) {
            maxOccurrences = std::max(maxOccurrences, occurrences);
        }

        matches[songID] = maxOccurrences;
    }

    return matches;
}

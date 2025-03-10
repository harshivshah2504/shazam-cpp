#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <header/mongo.h>
#include <header/utils.h>
#include <header/spectogram.h>
#include <header/fingerprint.h>
#include <header/mp3.h>


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


std::vector<Match> FindMatch(const std::vector<double>& audioSamples, long audioDuration, double sampleRate) {
    auto startTime = std::chrono::high_resolution_clock::now();


    auto spectrogram = Spectrogram(audioSamples, sampleRate);
    if (spectrogram.empty()) {
        throw std::runtime_error("Failed to generate spectrogram.");
    }


    auto peaks = ExtractPeaks(spectrogram, audioDuration);
    auto fingerprints = Fingerprint(peaks, GenerateUniqueID());

    std::vector<uint32_t> addresses;
    for (const auto& fp : fingerprints) {
        addresses.push_back(fp.first);
    }


    MongoClient db("mongodb://localhost:27017");
    if (!db.Connect()) {
        throw std::runtime_error("Database connection failed.");
    }


    auto matchesData = db.GetCouples(addresses);
    std::map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> matches;
    std::map<uint32_t, std::vector<uint32_t>> timestamps;

    for (const auto& [address, couples] : matchesData) {
        for (const auto& couple : couples) {
            matches[couple.songID].emplace_back(fingerprints[address].anchorTimeMs, couple.anchorTimeMs);
            timestamps[couple.songID].push_back(couple.anchorTimeMs);
        }
    }


    auto scores = analyzeRelativeTiming(matches);
    std::vector<Match> matchList;


    for (const auto& [songID, points] : scores) {
        auto song = db.GetSongByID(songID);
        if (!song) continue;

        std::sort(timestamps[songID].begin(), timestamps[songID].end());
        Match match(songID, song->title, song->artist, timestamps[songID][0], points);
        matchList.push_back(match);
    }


    std::sort(matchList.begin(), matchList.end(), [](const Match& a, const Match& b) {
        return a.score > b.score;
    });

    return matchList;
}


void findSongMatch(const std::string& filePath) {
    try {
 
        auto [samples, sampleRate, channels, duration] = decodeMP3ToFloat(filePath);
        if (samples.empty()) {
            throw std::runtime_error("Error converting MP3 bytes to samples.");
        }


        auto start = std::chrono::high_resolution_clock::now();
        std::vector<Match> matches = FindMatch(samples, duration, sampleRate);
        auto end = std::chrono::high_resolution_clock::now();


        std::chrono::duration<double> searchDuration = end - start;

        if (matches.empty()) {
            std::cout << "\nNo match found." << std::endl;
        } 
        else {  
            std::cout << "\nBest Match: " << matches[0].songTitle << " by " << matches[0].songArtist<< std::endl;
        }


        std::cout << "\nSearch took: " << searchDuration.count() << " seconds" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./shazam <audio_file_path>" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    findSongMatch(filePath);

    return 0;
}

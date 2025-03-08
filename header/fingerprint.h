#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <complex>
#include <header/models.h>

using namespace std;
std::unordered_map<uint32_t, Couple> Fingerprint(const std::vector<Peak>& peaks, uint32_t songID);

const int maxfreqBits = 9;
const int maxDeltaBits = 14;
const int targetZoneSize = 5;





uint32_t createAddress(const Peak& anchor, const Peak& target) {
    int anchorFreq = static_cast<int>((anchor.freq.real()));
    int targetFreq = static_cast<int>((target.freq.real()));
    uint32_t deltaMs = static_cast<uint32_t>((target.time - anchor.time) * 1000);


    return (static_cast<uint32_t>(anchorFreq) << (maxDeltaBits + maxfreqBits)) |
           (static_cast<uint32_t>(targetFreq) << maxDeltaBits) |
           deltaMs;
}


std::unordered_map<uint32_t, Couple> Fingerprint(const std::vector<Peak>& peaks, uint32_t songID){
    std::unordered_map<uint32_t, Couple> fingerprints;
    
    for (size_t i = 0; i < peaks.size(); i++) {
        const Peak& anchor = peaks[i];
        
        for (size_t j = i + 1; j < peaks.size() && j <= i + targetZoneSize; j++) {
            const Peak& target = peaks[j];
            uint32_t address = createAddress(anchor, target);
            uint32_t anchorTimeMs = static_cast<uint32_t>(anchor.time * 1000);
            
            fingerprints[address] = {anchorTimeMs, songID};
        }
    }
    
    return fingerprints;
}
// int main() {
//     vector<Peak> peaks = {{0.1, 300.0}, {0.2, 310.0}, {0.3, 320.0}, {0.4, 330.0}};
//     uint32_t songID = 1;
    
//     unordered_map<uint32_t, Couple> fingerprints = Fingerprint(peaks, songID);
    
//     cout << "Generated Fingerprints:" << endl;
//     for (const auto& [key, value] : fingerprints) {
//         cout << "Address: " << key << ", Anchor time: " << value.anchortimeMs << ", Song ID: " << value.songID << endl;
//     }
    
//     return 0;
// }
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <stdexcept>
#include <numeric> 
#include <fft.h>
#include <filter.h>
#include <models.h>

// Constants
const int DSP_RATIO = 4;
const int FREQ_BIN_SIZE = 1024;
const int MAX_FREQ = 5000;  // 5 kHz
const int HOP_SIZE = FREQ_BIN_SIZE / 32;

// Complex type for frequency domain data
using Complex = std::complex<double>;


// Downsample function
std::vector<double> Downsample(const std::vector<double>& input, int originalSampleRate, int targetSampleRate) {
    if (targetSampleRate <= 0 || originalSampleRate <= 0) {
        throw std::invalid_argument("Sample rates must be positive");
    }
    if (targetSampleRate > originalSampleRate) {
        throw std::invalid_argument("Target sample rate must be less than or equal to original sample rate");
    }

    int ratio = originalSampleRate / targetSampleRate;
    std::vector<double> resampled;
    
    for (size_t i = 0; i < input.size(); i += ratio) {
        int end = std::min(i + ratio, input.size());
        double sum = 0.0;
        for (int j = i; j < end; ++j) {
            sum += input[j];
        }
        resampled.push_back(sum / (end - i));
    }

    return resampled;
}


// Spectrogram function
std::vector<std::vector<Complex>> Spectrogram(const std::vector<double>& samples, int sampleRate) {
    LowPassFilter lpf(MAX_FREQ, static_cast<double>(sampleRate));
    std::vector<double> filteredSamples = lpf.filter(samples);

    std::vector<double> downsampledSamples = Downsample(filteredSamples, sampleRate, sampleRate / DSP_RATIO);
    int numOfWindows = downsampledSamples.size() / (FREQ_BIN_SIZE - HOP_SIZE);
    std::vector<std::vector<Complex>> spectrogram(numOfWindows);

    
    // Hamming window
    std::vector<double> window(FREQ_BIN_SIZE);
    for (int i = 0; i < FREQ_BIN_SIZE; ++i) {
        window[i] = 0.54 - 0.46 * cos(2 * M_PI * i / (FREQ_BIN_SIZE - 1));
    }

    // Perform STFT
    for (int i = 0; i < numOfWindows; ++i) {
        int start = i * HOP_SIZE;
        int end = start + FREQ_BIN_SIZE;
        if (end > downsampledSamples.size()) {
            end = downsampledSamples.size();
        }

        std::vector<double> bin(FREQ_BIN_SIZE, 0.0);
        for (int j = start; j < end; ++j) {
            bin[j - start] = downsampledSamples[j];
        }

        // Apply Hamming window
        for (size_t j = 0; j < bin.size(); ++j) {
            bin[j] *= window[j];
        }

        // Apply FFT
        spectrogram[i] = FFT(bin);
    }

    return spectrogram;
}

std::vector<Peak> ExtractPeaks(const std::vector<std::vector<Complex>>& spectrogram, double audioDuration) {
    if (spectrogram.empty()) {
        return {};
    }

    std::vector<Peak> peaks;
    double binDuration = audioDuration / spectrogram.size();

    // Frequency bands
    std::vector<std::pair<int, int>> bands = {{0, 10}, {10, 20}, {20, 40}, {40, 80}, {80, 160}, {160, 512}};

    for (size_t binIdx = 0; binIdx < spectrogram.size(); ++binIdx) {
        std::vector<double> maxMags;
        std::vector<Complex> maxFreqs;
        std::vector<double> freqIndices;

        // Analyze frequency bands
        for (const auto& band : bands) {
            double maxMag = 0.0;
            Complex maxFreq;
            int freqIdx = band.first;

            for (int idx = band.first; idx < band.second; ++idx) {
                double magnitude = std::abs(spectrogram[binIdx][idx]);
                if (magnitude > maxMag) {
                    maxMag = magnitude;
                    maxFreq = spectrogram[binIdx][idx];
                    freqIdx = idx;
                }
            }

            maxMags.push_back(maxMag);
            maxFreqs.push_back(maxFreq);
            freqIndices.push_back(static_cast<double>(freqIdx));
        }

        // Calculate average magnitude
        double maxMagsSum = 0.0;
        for (double mag : maxMags) {
            maxMagsSum += mag;
        }
        double avg = maxMagsSum / maxFreqs.size();

        // Add peaks
        for (size_t i = 0; i < maxMags.size(); ++i) {
            if (maxMags[i] > avg) {
                double peakTimeInBin = freqIndices[i] * binDuration / spectrogram[binIdx].size();
                double peakTime = binIdx * binDuration + peakTimeInBin;

                peaks.push_back(Peak{peakTime, maxFreqs[i]});
            }
        }
    }

    return peaks;
}

// int main() {
//     // Example usage
//     std::vector<double> samples = {0.0, 1.0, 0.0, -1.0};  // Example audio samples
//     int sampleRate = 44100;
//     double audioDuration = 1.0;  // 1 second

//     try {
//         auto spectrogram = Spectrogram(samples, sampleRate);
//         auto peaks = ExtractPeaks(spectrogram, audioDuration);

//         for (const auto& peak : peaks) {
//             std::cout << "Peak at time: " << peak.Time << "s, frequency: " << std::abs(peak.Freq) << "Hz\n";
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << '\n';
//     }

//     return 0;
// }

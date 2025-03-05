#include <cstdint>
#include <string>
#pragma once
#include <complex>

using Complex = std::complex<double>;

struct Peak {
    double time;
    Complex freq;
};


struct Couple {
    uint32_t anchorTimeMs;
    uint32_t songID;
};

struct RecordData {
    std::string audio;
    double duration;
    int channels;
    int sampleRate;
    int sampleSize;
};
#include <iostream>
#include <vector>
#include <tuple>
#include <mpg123.h>

#define BUFFER_SIZE 8192  
#define TARGET_SAMPLE_RATE 48000


std::tuple<std::vector<double>, long, int, double> decodeMP3ToFloat(const std::string& mp3FilePath) {
    std::vector<double> floatSamples;
    long sampleRate = 0;
    int channels = 0;
    double duration = 0.0;


    mpg123_init();
    mpg123_handle* mh = mpg123_new(NULL, NULL);
    if (mpg123_open(mh, mp3FilePath.c_str()) != MPG123_OK) {
        std::cerr << "Error opening MP3 file: " << mp3FilePath << std::endl;
        return {floatSamples, sampleRate, channels, duration}; 
    }

    int encoding;
    

    mpg123_format_none(mh);
    mpg123_format(mh, TARGET_SAMPLE_RATE, MPG123_STEREO, MPG123_ENC_SIGNED_16);


    mpg123_getformat(mh, &sampleRate, &channels, &encoding);

    if (encoding != MPG123_ENC_SIGNED_16) {
        std::cerr << "Unsupported encoding format!" << std::endl;
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return {floatSamples, sampleRate, channels, duration};
    }

    mpg123_scan(mh);  
    off_t totalFrames = mpg123_length(mh);

    if (totalFrames > 0 && sampleRate > 0) {
        duration = static_cast<double>(totalFrames) / static_cast<double>(sampleRate);
    }

    std::vector<unsigned char> buffer(BUFFER_SIZE);
    size_t done;


    while (mpg123_read(mh, buffer.data(), BUFFER_SIZE, &done) == MPG123_OK) {
        for (size_t i = 0; i < done; i += 2) { 
            int16_t sample = buffer[i] | (buffer[i + 1] << 8);
            floatSamples.push_back(sample / 32768.0); 
        }
    }


    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    return {floatSamples, sampleRate, channels, duration};
}

// // Main function to test MP3 decoding
// int main(int argc, char* argv[]) {
//     if (argc != 2) {
//         std::cerr << "Usage: " << argv[0] << " <mp3-file>" << std::endl;
//         return 1;
//     }

//     std::string filePath = argv[1];

//     // Call function and receive the result
//     auto [samples, sampleRate, channels, duration] = decodeMP3ToFloat(filePath);

//     if (samples.empty()) {
//         std::cerr << "Failed to decode MP3!" << std::endl;
//         return 1;
//     }

//     std::cout << "MP3 Decoded Successfully!" << std::endl;
//     std::cout << "Sample Rate: " << sampleRate << " Hz, Channels: " << channels << std::endl;
//     std::cout << "Duration: " << duration << " seconds" << std::endl;

//     // Print first 10 samples
//     for (size_t i = 0; i < 10 && i < samples.size(); i++) {
//         std::cout << "Sample[" << i << "] = " << samples[i] << std::endl;
//     }

//     return 0;
// }

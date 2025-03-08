#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

class LowPassFilter {
private:
    double alpha; 
    double yPrev;
public:

    LowPassFilter(double cutoffFrequency, double sampleRate) {
        double rc = 1.0 / (2 * M_PI * cutoffFrequency);
        double dt = 1.0 / sampleRate;
        alpha = dt / (rc + dt);
        yPrev = 0;
    }


    vector<double> filter(const vector<double>& input) {
        vector<double> filtered(input.size());
        for (size_t i = 0; i < input.size(); i++) {
            if (i == 0) {
                filtered[i] = input[i] * alpha;
            } else {
                filtered[i] = alpha * input[i] + (1 - alpha) * yPrev;
            }
            yPrev = filtered[i];
        }
        return filtered;
    }
};

// int main() {
//     vector<double> signal = {1.0, 2.0, 3.0, 4.0}; // Example input
//     double cutoffFrequency = 1.0; // Example cutoff frequency
//     double sampleRate = 10.0; // Example sample rate

//     LowPassFilter lpf(cutoffFrequency, sampleRate);
//     vector<double> filteredSignal = lpf.filter(signal);
    
//     return 0;
// }
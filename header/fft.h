#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace std;
using Complex = complex<double>;


vector<Complex> recursiveFFT(vector<Complex>& input) {
    int N = input.size();
    if (N <= 1) return input;
    
    vector<Complex> even(N / 2), odd(N / 2);
    for (int i = 0; i < N / 2; i++) {
        even[i] = input[i * 2];
        odd[i] = input[i * 2 + 1];
    }
    
    even = recursiveFFT(even);
    odd = recursiveFFT(odd);
    
    vector<Complex> fftResult(N);
    for (int k = 0; k < N / 2; k++) {
        Complex t = polar(1.0, -2 * M_PI * k / N) * odd[k];
        fftResult[k] = even[k] + t;
        fftResult[k + N / 2] = even[k] - t;
    }
    
    return fftResult;
}


vector<Complex> FFT(const vector<double>& input) {
    vector<Complex> complexInput(input.begin(), input.end());
    return recursiveFFT(complexInput);
}

// // Main function to test FFT
// int main() {
//     vector<double> signal = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}; // Example input
//     vector<Complex> fftResult = FFT(signal);

//     cout << "FFT Output:\n";
//     for (const auto& c : fftResult) {
//         cout << c << endl;
//     }

//     return 0;
// }
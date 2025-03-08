# Shazam-CPP

Shazam-CPP is a project aimed at creating a music recognition system similar to Shazam, implemented using C++ and related technologies.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Technologies Used](#technologies-used)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Overview

This project implements a music recognition system using an algorithm to identify songs from audio samples. The core logic is written in C++, with additional scripts and configurations in Makefile, CMake, Python, and TypeScript.

## Features

- **Audio Fingerprinting**: Extracts unique features from audio files.
- **Database Matching**: Matches audio fingerprints with a database of known songs.
- **Real-time Recognition**: Recognizes songs in real-time from audio input.

## Technologies Used

- **C++**: Main language for implementing the core logic.
- **C**: Utilized for certain performance-critical sections.
- **Makefile**: For automating the build process.
- **CMake**: For managing the build configuration.
- **Python**: Scripts for data preprocessing and analysis.
- **TypeScript**: Minimal usage for any web-based interfaces.

## Installation

1. **Clone the repository**:
    ```sh
    git clone https://github.com/harshivshah2504/shazam-cpp.git
    cd shazam-cpp
    ```

2. **Build the project**:
    ```sh
    mkdir build
    cd build
    cmake ..
    make
    ```

## Usage

1. **Run the application**:
    ```sh
    streamlit run app.py
    ```

2. **Provide an audio sample**: Follow the on-screen instructions to provide an audio sample for recognition.

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create a new branch (`git checkout -b feature-branch`).
3. Make your changes.
4. Commit your changes (`git commit -m 'Add some feature'`).
5. Push to the branch (`git push origin feature-branch`).
6. Open a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Resources and References
https://github.com/cgzirim/seek-tune/tree/main
https://drive.google.com/file/d/1ahyCTXBAZiuni6RTzHzLoOwwfTRFaU-C/view
https://hajim.rochester.edu/ece/sites/zduan/teaching/ece472/projects/2019/AudioFingerprinting.pdf
https://www.toptal.com/algorithms/shazam-it-music-processing-fingerprinting-and-recognition


## Acknowledgements

- Thanks to the original Shazam for the inspiration.
- [Libraries and frameworks used](#technologies-used).

# Shazam-CPP

Shazam-CPP is a project aimed at creating a music recognition system similar to Shazam, implemented using C++ and related technologies.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Technologies Used](#technologies-used)
- [Installation](#installation)
  - [Linux/Mac](#linuxmac)
  - [Windows](#windows)
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

### Linux/Mac

1. **Clone the repository**:
    ```sh
    git clone https://github.com/harshivshah2504/shazam-cpp.git
    cd shazam-cpp
    ```

2. **Install vcpkg**:
    ```sh
    git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
    cd ~/vcpkg
    ./bootstrap-vcpkg.sh
    export PATH=$HOME/vcpkg:$PATH
    echo 'export PATH=$HOME/vcpkg:$PATH' >> ~/.bashrc
    source ~/.bashrc
    ```

3. **Install Required Libraries**:
    ```sh
    vcpkg install mongo-cxx-driver boost-system boost-filesystem boost-thread
    ```

4. **Build the project**:
    ```sh
    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake ..
    make -j$(nproc)
    ```

### Windows

1. **Clone the repository**:
    ```sh
    git clone https://github.com/harshivshah2504/shazam-cpp.git
    cd shazam-cpp
    ```

2. **Install vcpkg**:
    ```powershell
    git clone https://github.com/microsoft/vcpkg.git C:/vcpkg
    cd C:/vcpkg
    .\bootstrap-vcpkg.bat
    ```

3. **Add vcpkg to System Path**:

4. **Install Required Libraries**:
    ```powershell
    vcpkg install mongo-cxx-driver:x64-windows
    vcpkg install boost-system boost-filesystem boost-thread:x64-windows
    ```

5. **Build the project**:
    ```powershell
    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ..
    cmake --build . --config Release
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

## Resources and References
1. https://github.com/cgzirim/seek-tune/tree/main
2. https://drive.google.com/file/d/1ahyCTXBAZiuni6RTzHzLoOwwfTRFaU-C/view
3. https://hajim.rochester.edu/ece/sites/zduan/teaching/ece472/projects/2019/AudioFingerprinting.pdf
4. https://www.toptal.com/algorithms/shazam-it-music-processing-fingerprinting-and-recognition

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Thanks to the original Shazam for the inspiration.
- [Libraries and frameworks used](#technologies-used).



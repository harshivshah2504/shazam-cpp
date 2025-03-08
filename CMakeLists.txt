cmake_minimum_required(VERSION 3.10)
project(SeekTune)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set paths for MongoDB C++ Driver (_noabi version)
set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake/mongocxx-4.0.0;/usr/local/lib/cmake/bsoncxx-4.0.0")
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

set(CMAKE_INSTALL_RPATH "/usr/local/lib")
set(CMAKE_BUILD_RPATH "/usr/local/lib")

# Explicitly find `_noabi` libraries
find_library(MONGOCXX_LIB mongocxx PATHS /usr/local/lib NO_DEFAULT_PATH)
find_library(BSONCXX_LIB bsoncxx PATHS /usr/local/lib NO_DEFAULT_PATH)

# Include directories
include_directories(. /usr/local/include /usr/local/include/mongocxx /usr/local/include/bsoncxx /usr/include/eigen3)

# Link directories
link_directories(/usr/local/lib)

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# Define source files for `add` executable
add_executable(add addSongs.cpp dbclient.cpp utils.cpp)
target_link_libraries(add 
    sndfile 
    ${MONGOCXX_LIB} 
    ${BSONCXX_LIB} 
    avformat 
    avcodec 
    avutil 
    swresample 
    Threads::Threads
    Boost::system
    uuid 
    mpg123
)

# Define source files for `shazam` executable
add_executable(shazam query.cpp utils.cpp)
target_link_libraries(shazam 
    sndfile 
    ${MONGOCXX_LIB} 
    ${BSONCXX_LIB} 
    avformat 
    avcodec 
    avutil 
    swresample 
    Threads::Threads
    Boost::system
    uuid 
    mpg123
)

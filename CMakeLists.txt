cmake_minimum_required(VERSION 3.10)
project(SeekTune)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(. /usr/local/include /usr/include/eigen3)

# Link directories
link_directories(/usr/local/lib)

# Find and link required libraries
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

# Define source files for `add` executable
add_executable(add addSongs.cpp dbclient.cpp)
target_link_libraries(add 
    audiorw 
    sndfile 
    mongocxx 
    bsoncxx 
    avformat 
    avcodec 
    avutil 
    swresample 
    ${CMAKE_THREAD_LIBS_INIT} 
    boost_system 
    uuid 
    mpg123
)

# Define source files for `shazam` executable
add_executable(shazam query.cpp)
target_link_libraries(shazam 
    audiorw 
    sndfile 
    mongocxx 
    bsoncxx 
    avformat 
    avcodec 
    avutil 
    swresample 
    ${CMAKE_THREAD_LIBS_INIT} 
    boost_system 
    uuid 
    mpg123
)

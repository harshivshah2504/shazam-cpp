cmake_minimum_required(VERSION 3.10)
project(SeekTune)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(. /usr/local/include /usr/include/eigen3)

# Link directories
link_directories(/usr/local/lib)

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# Ensure Boost is found
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found! Install Boost and try again.")
endif()
include_directories(${Boost_INCLUDE_DIRS})

# Define source files for `add` executable
add_executable(add addSongs.cpp dbclient.cpp utils.cpp)
target_link_libraries(add 
    audiorw 
    sndfile 
    mongocxx 
    bsoncxx 
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
    audiorw 
    sndfile 
    mongocxx 
    bsoncxx 
    avformat 
    avcodec 
    avutil 
    swresample 
    Threads::Threads
    Boost::system
    uuid 
    mpg123
)


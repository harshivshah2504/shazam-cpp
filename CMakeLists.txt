cmake_minimum_required(VERSION 3.10)
project(SeekTune)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set paths for MongoDB C++ Driver
set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake/mongocxx-4.0.0;/usr/local/lib/cmake/bsoncxx-4.0.0")
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

set(CMAKE_INSTALL_RPATH "/usr/local/lib")
set(CMAKE_BUILD_RPATH "/usr/local/lib")

find_library(MONGOCXX_LIB mongocxx PATHS /usr/local/lib NO_DEFAULT_PATH)
find_library(BSONCXX_LIB bsoncxx PATHS /usr/local/lib NO_DEFAULT_PATH)

# Include directories
include_directories(. /usr/local/include /usr/local/include/mongocxx /usr/local/include/bsoncxx /usr/include/eigen3)
link_directories(/usr/local/lib)

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found! Install Boost and try again.")
endif()
include_directories(${Boost_INCLUDE_DIRS})

# ✅ First, define executables before linking libraries
add_executable(add addSongs.cpp dbclient.cpp utils.cpp)
add_executable(shazam query.cpp utils.cpp)

# ✅ Now, link libraries to the defined executables
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

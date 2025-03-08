cmake_minimum_required(VERSION 3.10)
project(SeekTune)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set paths for MongoDB C++ Driver
set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake/mongocxx-4.0.0;/usr/local/lib/cmake/bsoncxx-4.0.0")
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

# Explicitly set RPATH settings
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:/usr/local/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Find the specific libraries - using the correct filename pattern
find_library(MONGOCXX_LIB NAMES libmongocxx.so._noabi mongocxx.so._noabi PATHS /usr/local/lib /home/vscode/mongo-driver/mongo-cxx-driver/build/src/mongocxx NO_DEFAULT_PATH)
find_library(BSONCXX_LIB NAMES libbsoncxx.so._noabi bsoncxx.so._noabi PATHS /usr/local/lib /home/vscode/mongo-driver/mongo-cxx-driver/build/src/bsoncxx NO_DEFAULT_PATH)

if(NOT MONGOCXX_LIB)
    message(FATAL_ERROR "mongocxx library not found! Check if it exists at /usr/local/lib or specify the correct path.")
endif()
if(NOT BSONCXX_LIB)
    message(FATAL_ERROR "bsoncxx library not found! Check if it exists at /usr/local/lib or specify the correct path.")
endif()

message(STATUS "Found mongocxx library: ${MONGOCXX_LIB}")
message(STATUS "Found bsoncxx library: ${BSONCXX_LIB}")

# Include directories
include_directories(. /usr/local/include /usr/local/include/mongocxx/v_noabi /usr/local/include/bsoncxx/v_noabi /usr/include/eigen3)

# Link directories
link_directories(/usr/local/lib /home/vscode/mongo-driver/mongo-cxx-driver/build/src/mongocxx /home/vscode/mongo-driver/mongo-cxx-driver/build/src/bsoncxx)

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
# Set RPATH for the add executable
set_target_properties(add PROPERTIES 
    INSTALL_RPATH "/usr/local/lib:/home/vscode/mongo-driver/mongo-cxx-driver/build/src/mongocxx:/home/vscode/mongo-driver/mongo-cxx-driver/build/src/bsoncxx"
    BUILD_WITH_INSTALL_RPATH TRUE
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
# Set RPATH for the shazam executable
set_target_properties(shazam PROPERTIES 
    INSTALL_RPATH "/usr/local/lib:/home/vscode/mongo-driver/mongo-cxx-driver/build/src/mongocxx:/home/vscode/mongo-driver/mongo-cxx-driver/build/src/bsoncxx"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# --------------------------
# 🔹 INSTALLATION COMMANDS
# --------------------------

# Install the binaries to /usr/local/bin
install(TARGETS add shazam
    RUNTIME DESTINATION /usr/local/bin
)

# Install MongoDB headers
install(DIRECTORY /usr/local/include/mongocxx
    DESTINATION /usr/local/include
)

install(DIRECTORY /usr/local/include/bsoncxx
    DESTINATION /usr/local/include
)

# Install shared libraries
install(FILES
    ${MONGOCXX_LIB}
    ${BSONCXX_LIB}
    DESTINATION /usr/local/lib
)

# Refresh ldconfig after install
install(CODE "execute_process(COMMAND ldconfig)")

message(STATUS "Installation setup complete. Run 'sudo make install' after build.")

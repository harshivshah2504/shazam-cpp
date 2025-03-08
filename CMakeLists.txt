cmake_minimum_required(VERSION 3.10)
project(SeekTune)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------
# 🔹 MONGODB DRIVER SETTINGS
# --------------------------

# Ensure CMake finds MongoDB C++ driver
set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake/mongocxx-4.0.0;/usr/local/lib/cmake/bsoncxx-4.0.0")

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

if(NOT mongocxx_FOUND OR NOT bsoncxx_FOUND)
    message(FATAL_ERROR "❌ MongoDB C++ driver not found! Check if installed in /usr/local/lib")
endif()

message(STATUS "✔ Found mongocxx: ${mongocxx_INCLUDE_DIRS}")
message(STATUS "✔ Found bsoncxx: ${bsoncxx_INCLUDE_DIRS}")

# --------------------------
# 🔹 INCLUDE & LINK DIRECTORIES
# --------------------------

include_directories(
    ${mongocxx_INCLUDE_DIRS}
    ${bsoncxx_INCLUDE_DIRS}
    /usr/include/eigen3
    ${CMAKE_SOURCE_DIR}  # Ensure local headers are included
)

# --------------------------
# 🔹 DEPENDENCIES
# --------------------------

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# --------------------------
# 🔹 BUILD EXECUTABLES
# --------------------------

# 🎵 ADD EXECUTABLE
# 🎵 ADD EXECUTABLE
add_executable(add addSongs.cpp dbclient.cpp utils.cpp)
target_link_libraries(add 
    PRIVATE
    mongocxx  # 🔥 Use this instead of mongocxx_shared
    bsoncxx   # 🔥 Use this instead of bsoncxx_shared
    sndfile
    avformat avcodec avutil swresample
    Threads::Threads
    Boost::system
    uuid mpg123
)

set_target_properties(add PROPERTIES 
    INSTALL_RPATH "/usr/local/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)


# 🎵 SHAZAM EXECUTABLE
add_executable(shazam query.cpp utils.cpp)
target_link_libraries(shazam 
    PRIVATE
    mongocxx  # 🔥 Use this instead of mongocxx_shared
    bsoncxx   # 🔥 Use this instead of bsoncxx_shared
    sndfile
    avformat avcodec avutil swresample
    Threads::Threads
    Boost::system
    uuid mpg123
)

set_target_properties(shazam PROPERTIES 
    INSTALL_RPATH "/usr/local/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# --------------------------
# 🔹 INSTALLATION COMMANDS
# --------------------------

# Install binaries
install(TARGETS add shazam
    RUNTIME DESTINATION /usr/local/bin
)

# Install MongoDB headers
install(DIRECTORY /usr/local/include/mongocxx /usr/local/include/bsoncxx
    DESTINATION /usr/local/include
)

# Install MongoDB shared libraries
install(FILES
    /usr/local/lib/libmongocxx.so
    /usr/local/lib/libbsoncxx.so
    DESTINATION /usr/local/lib
)

# Run ldconfig to refresh shared library cache
install(CODE "execute_process(COMMAND ldconfig)")

message(STATUS "✔ Installation setup complete. Run 'sudo make install' after build.")

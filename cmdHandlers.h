#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <sstream>
#include <thread>
#include <mutex>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <nlohmann/json.hpp>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/ws_listener.h>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// Constants
const std::string SONGS_DIR = "songs";

// Logger utility
void logMessage(const std::string& message) {
    std::cout << "[LOG] " << message << std::endl;
}

// Function to create a directory
bool createFolder(const std::string& path) {
    return fs::create_directories(path);
}

// Function to delete a file
bool deleteFile(const std::string& path) {
    return fs::remove(path);
}

// Function to generate a random unique ID
uint32_t generateUniqueID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis;
    return dis(gen);
}

// WebSocket server for real-time communication
class WebSocketServer {
public:
    WebSocketServer(const std::string& address) : listener(address) {
        listener.support([this](websocket_incoming_message msg) {
            msg.extract_string().then([this](std::string body) {
                logMessage("Received message: " + body);
                handleMessage(body);
            });
        });
    }

    void start() {
        listener.open().then([this]() { logMessage("WebSocket server started!"); }).wait();
    }

    void stop() {
        listener.close().wait();
    }

private:
    websocket_listener listener;

    void handleMessage(const std::string& message) {
        logMessage("Processing message: " + message);
    }
};

// HTTP Server to handle song-related requests
class HttpServer {
public:
    HttpServer(const std::string& address) : listener(address) {
        listener.support(methods::GET, [this](http_request request) {
            handleGetRequest(request);
        });

        listener.support(methods::POST, [this](http_request request) {
            handlePostRequest(request);
        });
    }

    void start() {
        listener.open().then([this]() { logMessage("HTTP server started!"); }).wait();
    }

    void stop() {
        listener.close().wait();
    }

private:
    http_listener listener;

    void handleGetRequest(http_request request) {
        logMessage("Received GET request: " + request.request_uri().to_string());
        request.reply(status_codes::OK, json::object({{"message", "GET request received"}}).dump());
    }

    void handlePostRequest(http_request request) {
        request.extract_json().then([this, request](json::value body) {
            logMessage("Received POST request: " + body.serialize());
            request.reply(status_codes::OK, json::object({{"message", "POST request received"}}).dump());
        }).wait();
    }
};

// Function to process and store a song
bool processAndSaveSong(const std::string& filePath, const std::string& title, const std::string& artist, const std::string& ytID) {
    logMessage("Processing song: " + title + " by " + artist);

    std::string fileName = fs::path(filePath).stem().string();
    std::string wavFile = fileName + ".wav";
    std::string newFilePath = SONGS_DIR + "/" + wavFile;

    if (!createFolder(SONGS_DIR)) {
        logMessage("Failed to create directory: " + SONGS_DIR);
        return false;
    }

    fs::rename(filePath, newFilePath);
    logMessage("Saved song to " + newFilePath);

    return true;
}

// Function to erase the database and delete song files
void eraseDatabase() {
    logMessage("Erasing database and song files...");

    for (const auto& entry : fs::directory_iterator(SONGS_DIR)) {
        if (entry.path().extension() == ".wav" || entry.path().extension() == ".m4a") {
            fs::remove(entry.path());
        }
    }

    logMessage("Erase complete!");
}

// Function to start the server
void serve(const std::string& protocol, const std::string& port) {
    std::string address = protocol + "://localhost:" + port;
    HttpServer httpServer(address);
    WebSocketServer wsServer("ws://localhost:" + port + "/ws");

    std::thread httpThread([&httpServer]() { httpServer.start(); });
    std::thread wsThread([&wsServer]() { wsServer.start(); });

    httpThread.join();
    wsThread.join();
}

// Main function
int main() {
    logMessage("Starting server...");

    std::thread serverThread([]() { serve("http", "8080"); });

    serverThread.join();
    return 0;
}

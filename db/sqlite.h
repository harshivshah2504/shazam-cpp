#ifndef SQLITE_DB_CLIENT_H
#define SQLITE_DB_CLIENT_H

#include "client.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <iostream>
#include <stdexcept>
#include <chrono>

class SQLiteClient : public DBClient {
private:
    sqlite3* db;
    bool connected;
    
public:
    SQLiteClient(const std::string& dataSourceName) : db(nullptr), connected(false) {}
    
    bool Connect() override {
        if (connected) return true;
        
        int rc = sqlite3_open("db.sqlite3", &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            db = nullptr;
            return false;
        }
        
        if (!createTables()) {
            sqlite3_close(db);
            db = nullptr;
            return false;
        }
        
        connected = true;
        return true;
    }
    
    void Disconnect() override {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
        connected = false;
    }
    
    bool IsConnected() const override {
        return connected;
    }
    
    bool StoreFingerprints(const std::unordered_map<uint32_t, Couple>& fingerprints) override {
        if (!connected) return false;
        
        // Begin transaction
        if (executeSQL("BEGIN TRANSACTION") != SQLITE_OK) {
            return false;
        }
        
        // Prepare statement
        const char* query = "INSERT OR REPLACE INTO fingerprints (address, anchorTimeMs, songID) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
            executeSQL("ROLLBACK");
            return false;
        }
        
        // Insert fingerprints
        for (const auto& [address, couple] : fingerprints) {
            sqlite3_reset(stmt);
            sqlite3_bind_int(stmt, 1, address);
            sqlite3_bind_int(stmt, 2, couple.anchorTimeMs);
            sqlite3_bind_int(stmt, 3, couple.songID);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Error executing statement: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_finalize(stmt);
                executeSQL("ROLLBACK");
                return false;
            }
        }
        
        sqlite3_finalize(stmt);
        return executeSQL("COMMIT") == SQLITE_OK;
    }
    
    std::map<uint32_t, std::vector<Couple>> GetCouples(const std::vector<uint32_t>& addresses) override {
        std::map<uint32_t, std::vector<Couple>> result;
        
        if (!connected) return result;
        
        for (const auto& address : addresses) {
            std::vector<Couple> couples;
            
            std::string query = "SELECT anchorTimeMs, songID FROM fingerprints WHERE address = ?";
            sqlite3_stmt* stmt;
            
            if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
                continue;
            }
            
            sqlite3_bind_int(stmt, 1, address);
            
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Couple couple;
                couple.anchorTimeMs = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
                couple.songID = static_cast<uint32_t>(sqlite3_column_int(stmt, 1));
                couples.push_back(couple);
            }
            
            sqlite3_finalize(stmt);
            
            if (!couples.empty()) {
                result[address] = couples;
            }
        }
        
        return result;
    }
    
    int TotalSongs() override {
        if (!connected) return 0;
        
        const char* query = "SELECT COUNT(*) FROM songs";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error preparing count statement: " << sqlite3_errmsg(db) << std::endl;
            return 0;
        }
        
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count;
    }
    
    uint32_t RegisterSong(const std::string& songTitle, const std::string& songArtist, const std::string& ytID) override {
        if (!connected) return 0;
        
        // Begin transaction
        if (executeSQL("BEGIN TRANSACTION") != SQLITE_OK) {
            return 0;
        }
        
        // Generate a unique ID and song key
        uint32_t songID = generateUniqueID();
        std::string songKey = generateSongKey(songTitle, songArtist);
        
        // Prepare statement
        const char* query = "INSERT INTO songs (id, title, artist, ytID, key) VALUES (?, ?, ?, ?, ?)";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
            executeSQL("ROLLBACK");
            return 0;
        }
        
        // Bind parameters
        sqlite3_bind_int(stmt, 1, songID);
        sqlite3_bind_text(stmt, 2, songTitle.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, songArtist.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, ytID.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, songKey.c_str(), -1, SQLITE_STATIC);
        
        // Execute statement
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc != SQLITE_DONE) {
            std::cerr << "Error inserting song: " << sqlite3_errmsg(db) << std::endl;
            
            // Check if it's a constraint violation (duplicate key or ID)
            if (sqlite3_extended_errcode(db) == SQLITE_CONSTRAINT_UNIQUE) {
                std::cerr << "Song with ytID or key already exists" << std::endl;
            }
            
            executeSQL("ROLLBACK");
            return 0;
        }
        
        if (executeSQL("COMMIT") != SQLITE_OK) {
            return 0;
        }
        
        return songID;
    }
    
    std::optional<Song> GetSong(const std::string& filterKey, const std::string& value) override {
        if (!connected) return std::nullopt;
        
        // Valid filter keys
        const std::string validKeys = "id | ytID | key";
        if (validKeys.find(filterKey) == std::string::npos) {
            std::cerr << "Invalid filter key: " << filterKey << std::endl;
            return std::nullopt;
        }
        
        std::string query = "SELECT title, artist, ytID FROM songs WHERE " + filterKey + " = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
            return std::nullopt;
        }
        
        // Bind parameter based on type
        if (filterKey == "id") {
            sqlite3_bind_int(stmt, 1, std::stoi(value));
        } else {
            sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_STATIC);
        }
        
        Song song;
        bool found = false;
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            song.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            song.artist = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            song.youTubeID = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            found = true;
        }
        
        sqlite3_finalize(stmt);
        
        if (found) {
            return song;
        }
        
        return std::nullopt;
    }
    
    std::optional<Song> GetSongByID(uint32_t songID) override {
        return GetSong("id", std::to_string(songID));
    }
    
    std::optional<Song> GetSongByYTID(const std::string& ytID) override {
        return GetSong("ytID", ytID);
    }
    
    std::optional<Song> GetSongByKey(const std::string& key) override {
        return GetSong("key", key);
    }
    
    bool DeleteSongByID(uint32_t songID) override {
        if (!connected) return false;
        
        const char* query = "DELETE FROM songs WHERE id = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error preparing delete statement: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        sqlite3_bind_int(stmt, 1, songID);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }
    
    bool DeleteCollection(const std::string& collectionName) override {
        if (!connected) return false;
        
        std::string query = "DROP TABLE IF EXISTS " + collectionName;
        return executeSQL(query.c_str()) == SQLITE_OK;
    }
    
private:
    int executeSQL(const char* sql) {
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        return rc;
    }
    
    bool createTables() {
        const char* createSongsTable = 
            "CREATE TABLE IF NOT EXISTS songs ("
            "    id INTEGER PRIMARY KEY,"
            "    title TEXT NOT NULL,"
            "    artist TEXT NOT NULL,"
            "    ytID TEXT UNIQUE,"
            "    key TEXT NOT NULL UNIQUE"
            ");";
        
        const char* createFingerprintsTable = 
            "CREATE TABLE IF NOT EXISTS fingerprints ("
            "    address INTEGER NOT NULL,"
            "    anchorTimeMs INTEGER NOT NULL,"
            "    songID INTEGER NOT NULL,"
            "    PRIMARY KEY (address, anchorTimeMs, songID)"
            ");";
        
        if (executeSQL(createSongsTable) != SQLITE_OK) {
            std::cerr << "Error creating songs table" << std::endl;
            return false;
        }
        
        if (executeSQL(createFingerprintsTable) != SQLITE_OK) {
            std::cerr << "Error creating fingerprints table" << std::endl;
            return false;
        }
        
        return true;
    }
    
    uint32_t generateUniqueID() {
        // Simple implementation - in production, use a more robust method
        return static_cast<uint32_t>(std::hash<std::string>{}(
            std::to_string(std::chrono::system_clock::now().time_since_epoch().count())));
    }
    
    std::string generateSongKey(const std::string& title, const std::string& artist) {
        return title + "---" + artist;
    }
};

#endif // SQLITE_DB_CLIENT_H
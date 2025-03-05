#ifndef DB_CLIENT_H
#define DB_CLIENT_H

#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <optional>
#include <memory>
#include <cstdint>
#include "./models.h"

// Song struct to match the Go implementation
struct Song {
    std::string title;
    std::string artist;
    std::string youTubeID;
};

// DBClient interface that mimics the Go interface
class DBClient {
public:
    virtual ~DBClient() = default;
    
    virtual bool Connect() = 0;
    virtual void Disconnect() = 0;
    virtual bool IsConnected() const = 0;
    
    virtual bool StoreFingerprints(const std::unordered_map<uint32_t, Couple>& fingerprints) = 0;
    virtual std::map<uint32_t, std::vector<Couple>> GetCouples(const std::vector<uint32_t>& addresses) = 0;
    
    virtual int TotalSongs() = 0;
    virtual uint32_t RegisterSong(const std::string& songTitle, const std::string& songArtist, const std::string& ytID) = 0;
    
    virtual std::optional<Song> GetSong(const std::string& filterKey, const std::string& value) = 0;
    virtual std::optional<Song> GetSongByID(uint32_t songID) = 0;
    virtual std::optional<Song> GetSongByYTID(const std::string& ytID) = 0;
    virtual std::optional<Song> GetSongByKey(const std::string& key) = 0;
    
    virtual bool DeleteSongByID(uint32_t songID) = 0;
    virtual bool DeleteCollection(const std::string& collectionName) = 0;
};

// Factory function to create the appropriate DB client
std::unique_ptr<DBClient> NewDBClient();

#endif // DB_CLIENT_H
#ifndef MONGO_DB_CLIENT_H
#define MONGO_DB_CLIENT_H

#include "client.h"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/string/to_string.hpp> 
#include <bsoncxx/builder/stream/document.hpp>
#include <string>
#include <vector>
#include <map>
#include <optional>



class MongoClient : public DBClient {
private:
    uint32_t num = 1;
    mongocxx::client client;
    mongocxx::database db;
    bool connected;
    
    // Singleton instance for MongoDB client
    static mongocxx::instance& getInstance() {
        static mongocxx::instance instance{}; // This initializes the MongoDB driver
        return instance;
    }
    
public:
    MongoClient(const std::string& uri) : connected(false) {
        // Make sure the MongoDB instance is initialized
        getInstance();
    }
    
    bool Connect() override {
        try {
            mongocxx::uri uri(getConnectionUri());
            client = mongocxx::client(uri);
            db = client["song-recognition"];
            connected = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error connecting to MongoDB: " << e.what() << std::endl;
            return false;
        }
    }
    
    void Disconnect() override {
        // The MongoDB C++ driver handles disconnection in the destructor
        connected = false;
    }
    
    bool IsConnected() const override {
        return connected;
    }
    
    bool StoreFingerprints(const std::unordered_map<uint32_t, Couple>& fingerprints) override {
        if (!connected) return false;
        
        auto collection = db["fingerprints"];
        
        try {
            for (const auto& [address, couple] : fingerprints) {
                using namespace bsoncxx::builder::stream;
                
                document filter_builder, update_builder;
                filter_builder << "_id" << static_cast<int64_t>(address);
                
                update_builder << "$push" << open_document
                              << "couples" << open_document
                              << "anchorTimeMs" << static_cast<int64_t>(couple.anchorTimeMs)
                              << "songID" << static_cast<int64_t>(couple.songID)
                              << close_document
                              << close_document;
                
                mongocxx::options::update options;
                options.upsert(true);
                
                collection.update_one(filter_builder.view(), update_builder.view(), options);
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error storing fingerprints: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::map<uint32_t, std::vector<Couple>> GetCouples(const std::vector<uint32_t>& addresses) override {
        std::map<uint32_t, std::vector<Couple>> result;
        
        if (!connected) return result;
        
        auto collection = db["fingerprints"];
        
        for (const auto& address : addresses) {
            try {
                using namespace bsoncxx::builder::stream;
                document filter_builder;
                filter_builder << "_id" << static_cast<int64_t>(address);
                
                auto doc = collection.find_one(filter_builder.view());
                if (doc) {
                    auto doc_view = doc->view();
                    auto couples_array = doc_view["couples"].get_array().value;
                    
                    std::vector<Couple> couples;
                    for (const auto& element : couples_array) {
                        auto couple_doc = element.get_document().value;
                        Couple couple;
                        couple.anchorTimeMs = static_cast<uint32_t>(couple_doc["anchorTimeMs"].get_int64().value);
                        couple.songID = static_cast<uint32_t>(couple_doc["songID"].get_int64().value);
                        couples.push_back(couple);
                    }
                    
                    result[address] = couples;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error retrieving couples for address " << address << ": " << e.what() << std::endl;
            }
        }
        
        return result;
    }
    
    int TotalSongs() override {
        if (!connected) return 0;
        
        try {
            auto collection = db["songs"];
            return static_cast<int>(collection.count_documents({}));
        } catch (const std::exception& e) {
            std::cerr << "Error counting songs: " << e.what() << std::endl;
            return 0;
        }
    }
    
    uint32_t RegisterSong(const std::string& songTitle, const std::string& songArtist, const std::string& ytID) override {
        if (!connected) return 0;

        try {
            auto collection = db["songs"];
            
            // Create an index on the "key" field to ensure it is unique
            bsoncxx::builder::stream::document index_builder;
            index_builder << "key" << 1;  // Create an index on the "key" field
            bsoncxx::document::view_or_value index_doc{index_builder.view()};
            mongocxx::options::index index_options;
            index_options.unique(true);  // Ensure uniqueness for the "key" field
            collection.create_index(index_doc, index_options);
            auto result = collection.find_one({}, mongocxx::options::find{}
            .sort(bsoncxx::builder::stream::document{} << "_id" << -1 << bsoncxx::builder::stream::finalize));
            int32_t songID = 1;  // Default value
                    if (result) {
                        auto doc = result->view();
                        if (doc["_id"].type() == bsoncxx::type::k_int32) {
                            songID = doc["_id"].get_int32() + 1;
                        } else if (doc["_id"].type() == bsoncxx::type::k_int64) {
                            songID = static_cast<int32_t>(doc["_id"].get_int64() + 1);
                        }
                    }
            std::string key = generateSongKey(songTitle, songArtist);

            // Insert the song document with the unique ID and key
            bsoncxx::builder::stream::document doc_builder;
            std::cout << "SongID: " << songID << std::endl;
            std::cout << "key: " << key << std::endl;
            std::cout << "ytID: " << ytID << std::endl;
            doc_builder << "_id" << static_cast<int64_t>(songID)
                        << "key" << key
                        << "ytID" << ytID;

            collection.insert_one(doc_builder.view());
            return songID;

        } catch (const mongocxx::exception& e) {
            std::cerr << "Error registering song: " << e.what() << std::endl;
            // Check for duplicate key error
            if (e.code().value() == 11000) {  // MongoDB duplicate key error code
                std::cerr << "Duplicate entry detected for ytID or key: " << ytID << " or " << generateSongKey(songTitle, songArtist) << std::endl;
            }
            return 0;
        }
    }
    
    std::optional<Song> GetSong(const std::string& filterKey, const std::string& value) override {
        if (!connected) return std::nullopt;
        
        // Valid filter keys
        const std::string validKeys = "_id | ytID | key";
        if (validKeys.find(filterKey) == std::string::npos) {
            std::cerr << "Invalid filter key: " << filterKey << std::endl;
            return std::nullopt;
        }
        
        try {
            auto collection = db["songs"];
            
            using namespace bsoncxx::builder::stream;
            document filter_builder;
            
            if (filterKey == "_id") {
                try {
                    filter_builder << filterKey << std::stoi(value);  // Attempt to convert value to an integer
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid argument: " << value << " is not a valid integer." << std::endl;
                    // Handle the error or set a default value if needed
                } catch (const std::out_of_range& e) {
                    std::cerr << "Out of range: " << value << " is too large for an integer." << std::endl;
                    // Handle the error or set a default value if needed
                }
            } else {
                filter_builder << filterKey << value;  // Proceed as usual for other filter keys
            }

            
            auto doc = collection.find_one(filter_builder.view());
            if (doc) {
                auto doc_view = doc->view();

                // Ensure the fields exist before accessing them
                bsoncxx::document::element ytID_elem = doc_view["ytID"];
                bsoncxx::document::element key_elem = doc_view["key"];

                if (ytID_elem && ytID_elem.type() == bsoncxx::type::k_string &&
                    key_elem && key_elem.type() == bsoncxx::type::k_string) {
                    
                    // Retrieve string values correctly using bsoncxx::string::to_string
                    std::string ytID = bsoncxx::string::to_string(ytID_elem.get_string().value);
                    std::string key = bsoncxx::string::to_string(key_elem.get_string().value);

                    // Parse the key to get title and artist
                    size_t separatorPos = key.find("---");
                    std::string title = (separatorPos != std::string::npos) ? key.substr(0, separatorPos) : key;
                    std::string artist = (separatorPos != std::string::npos) ? key.substr(separatorPos + 3) : "";

                    return Song{title, artist, ytID};  // Construct and return the Song object
                }
            }

        } catch (const std::exception& e) {
            std::cerr << "Error retrieving song: " << e.what() << std::endl;
        }
        
        return std::nullopt;
    }
    
    std::optional<Song> GetSongByID(uint32_t songID) override {
        return GetSong("_id", std::to_string(songID));
    }
    
    std::optional<Song> GetSongByYTID(const std::string& ytID) override {
        return GetSong("ytID", ytID);
    }
    
    std::optional<Song> GetSongByKey(const std::string& key) override {
        return GetSong("key", key);
    }
    
    bool DeleteSongByID(uint32_t songID) override {
        if (!connected) return false;
        
        try {
            auto collection = db["songs"];
            
            using namespace bsoncxx::builder::stream;
            document filter_builder;
            filter_builder << "_id" << static_cast<int64_t>(songID);
            
            collection.delete_one(filter_builder.view());
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error deleting song: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool DeleteCollection(const std::string& collectionName) override {
        if (!connected) return false;
        
        try {
            db[collectionName].drop();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error dropping collection: " << e.what() << std::endl;
            return false;
        }
    }
    
private:
    std::string getConnectionUri() {
        // Get environment variables for MongoDB connection
        std::string dbUsername = getEnv("DB_USER", "");
        std::string dbPassword = getEnv("DB_PASS", "");
        std::string dbName = getEnv("DB_NAME", "");
        std::string dbHost = getEnv("DB_HOST", "");
        std::string dbPort = getEnv("DB_PORT", "");
        
        if (dbUsername.empty() || dbPassword.empty()) {
            return "mongodb://localhost:27017";
        }
        
        return "mongodb://" + dbUsername + ":" + dbPassword + "@" + 
               dbHost + ":" + dbPort + "/" + dbName;
    }
    
    std::string getEnv(const std::string& key, const std::string& defaultValue = "") {
        const char* value = std::getenv(key.c_str());
        return value ? value : defaultValue;
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

#endif // MONGO_DB_CLIENT_H
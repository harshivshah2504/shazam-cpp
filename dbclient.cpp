#include "client.h"
#include "mongo.h"
#include <cstdlib>  // For getenv
#include <iostream>
#include <memory>

std::unique_ptr<DBClient> NewDBClient() {
    const char* mongoUri = std::getenv("MONGO_URI");  // Get the environment variable

    if (!mongoUri) {
        std::cerr << "Error: MONGO_URI environment variable not set!" << std::endl;
        return nullptr;  // Return null if the variable isn't set
    }

    return std::make_unique<MongoClient>(mongoUri);  // Use env variable
}

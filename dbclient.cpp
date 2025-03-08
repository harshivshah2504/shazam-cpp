#include "client.h"
#include "mongo.h"  // Include the file where MongoClient is defined

std::unique_ptr<DBClient> NewDBClient() {
    // Return a MongoClient instance, but you can modify this for other DB clients if needed
    return std::make_unique<MongoClient>("mongodb://localhost:27017");
}

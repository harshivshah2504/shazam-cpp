#include <header/client.h>
#include <header/mongo.h>

std::unique_ptr<DBClient> NewDBClient() {
    return std::make_unique<MongoClient>("mongodb://localhost:27017");
}

#ifndef SPOTIFY_H
#define SPOTIFY_H

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <optional>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <cpprest/filestream.h>

using namespace web;
using namespace web::http;
using namespace web::http::client;

struct Track {
    std::string title;
    std::string artist;
    std::string album;
    int duration;
};

class Spotify {
public:
    std::string getAccessToken();
    std::string extractID(const std::string& url);
    web::json::value sendRequest(const std::string& endpoint);

    std::optional<Track> getTrackInfo(const std::string& url);
    std::vector<Track> getPlaylistInfo(const std::string& url);
    std::vector<Track> getAlbumInfo(const std::string& url);
};

#endif // SPOTIFY_H

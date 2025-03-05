#include "spotify.h"

const std::string TOKEN_ENDPOINT = "https://open.spotify.com/get_access_token?reason=transport&productType=web-player";
const std::string TRACK_ENDPOINT = "https://api-partner.spotify.com/pathfinder/v1/query?operationName=getTrack&variables=";
const std::string PLAYLIST_ENDPOINT = "https://api-partner.spotify.com/pathfinder/v1/query?operationName=fetchPlaylist&variables=";
const std::string ALBUM_ENDPOINT = "https://api-partner.spotify.com/pathfinder/v1/query?operationName=getAlbum&variables=";

std::string Spotify::getAccessToken() {
    web::http::client::http_client client(U(TOKEN_ENDPOINT));
    web::http::http_request request(web::http::methods::GET);

    web::http::http_response response = client.request(request).get();
    if (response.status_code() != web::http::status_codes::OK) {
        throw std::runtime_error("Failed to fetch access token.");
    }

    web::json::value json_response = response.extract_json().get();
    return json_response[U("accessToken")].as_string();
}

std::string Spotify::extractID(const std::string& url) {
    std::regex pattern(R"(https:\/\/open\.spotify\.com\/(track|album|playlist)\/([a-zA-Z0-9]+))");
    std::smatch match;
    if (std::regex_search(url, match, pattern) && match.size() > 2) {
        return match[2].str();
    }
    throw std::runtime_error("Invalid Spotify URL.");
}

web::json::value Spotify::sendRequest(const std::string& endpoint) {
    std::string accessToken = getAccessToken();

    web::http::client::http_client client(U(endpoint));
    web::http::http_request request(web::http::methods::GET);
    request.headers().add(U("Authorization"), U("Bearer " + accessToken));

    web::http::http_response response = client.request(request).get();
    if (response.status_code() != web::http::status_codes::OK) {
        throw std::runtime_error("Failed to fetch data from Spotify.");
    }

    return response.extract_json().get();
}

std::optional<Track> Spotify::getTrackInfo(const std::string& url) {
    std::string id = extractID(url);
    std::string endpoint = TRACK_ENDPOINT + "{\"uri\":\"spotify:track:" + id + "\"}";

    web::json::value jsonResponse = sendRequest(endpoint);
    
    if (!jsonResponse.has_field(U("data"))) {
        return std::nullopt;
    }

    Track track;
    track.title = jsonResponse[U("data")][U("trackUnion")][U("name")].as_string();
    track.artist = jsonResponse[U("data")][U("trackUnion")][U("firstArtist")][U("items")][0][U("profile")][U("name")].as_string();
    track.album = jsonResponse[U("data")][U("trackUnion")][U("albumOfTrack")][U("name")].as_string();
    track.duration = jsonResponse[U("data")][U("trackUnion")][U("duration")][U("totalMilliseconds")].as_integer() / 1000;

    return track;
}

std::vector<Track> Spotify::getPlaylistInfo(const std::string& url) {
    std::string id = extractID(url);
    std::string endpoint = PLAYLIST_ENDPOINT + "{\"uri\":\"spotify:playlist:" + id + "\"}";

    web::json::value jsonResponse = sendRequest(endpoint);
    std::vector<Track> tracks;

    auto items = jsonResponse[U("data")][U("playlistV2")][U("content")][U("items")].as_array();
    for (const auto& item : items) {
        Track track;
        track.title = item.at(U("itemV2")).at(U("data")).at(U("name")).as_string();
        auto artistsArray = item.at(U("itemV2")).at(U("data")).at(U("artists")).at(U("items")).as_array();
        if (!artistsArray.size()>0) {
            track.artist = artistsArray[0].at(U("profile")).at(U("name")).as_string();
        }       
        track.album = item.at(U("itemV2")).at(U("data")).at(U("albumOfTrack")).at(U("name")).as_string();
        track.duration = item.at(U("itemV2")).at(U("data")).at(U("trackDuration")).at(U("totalMilliseconds")).as_integer() / 1000;

        tracks.push_back(track);
    }

    return tracks;
}

std::vector<Track> Spotify::getAlbumInfo(const std::string& url) {
    std::string id = extractID(url);
    std::string endpoint = ALBUM_ENDPOINT + "{\"uri\":\"spotify:album:" + id + "\"}";

    web::json::value jsonResponse = sendRequest(endpoint);
    std::vector<Track> tracks;

    auto items = jsonResponse[U("data")][U("albumUnion")][U("tracks")][U("items")].as_array();
    for (const auto& item : items) {
        Track track;
        track.title = item.at(U("track")).at(U("name")).as_string();
        auto artistsArray = item.at(U("track")).at(U("artists")).at(U("items")).as_array();
        if (!artistsArray.size()>0) {
            track.artist = artistsArray[0].at(U("profile")).at(U("name")).as_string();
        }

        track.album = jsonResponse.at(U("data")).at(U("albumUnion")).at(U("name")).as_string();
        track.duration = item.at(U("track")).at(U("duration")).at(U("totalMilliseconds")).as_integer() / 1000;


        tracks.push_back(track);
    }

    return tracks;
}

#ifndef YOUTUBE_SEARCH_H
#define YOUTUBE_SEARCH_H

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cpprest/http_client.h>
#include <cpprest/json.h>

struct SearchResult {
    std::string title;
    std::string uploader;
    std::string url;
    std::string duration;
    std::string id;
    bool live;
};

class YouTubeSearch {
public:
    static std::vector<SearchResult> ytSearch(const std::string& query, int limit = 10);
    static std::string GetYouTubeId(const std::string& query);
    static std::string urlEncode(const std::string& value);
};

#endif // YOUTUBE_SEARCH_H

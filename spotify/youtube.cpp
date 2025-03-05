#include "youtube.h"
#include <cpprest/uri.h>
#include <cpprest/http_client.h>
#include <regex>

using namespace web;
using namespace web::http;
using namespace web::http::client;

// Encode URL parameters properly
std::string YouTubeSearch::urlEncode(const std::string& value) {
    return uri::encode_data_string(value);
}

// Perform YouTube search using Google search
std::vector<SearchResult> YouTubeSearch::ytSearch(const std::string& query, int limit) {
    std::vector<SearchResult> results;
    std::string ytSearchUrl = "https://www.youtube.com/results?search_query=" + urlEncode(query);

    http_client client(U(ytSearchUrl));
    http_request request(methods::GET);

    try {
        http_response response = client.request(request).get();
        if (response.status_code() != status_codes::OK) {
            throw std::runtime_error("Failed to fetch YouTube search results.");
        }

        std::string responseBody = response.extract_string().get();
        
        // Regex pattern to extract YouTube video IDs
        std::regex videoRegex(R"(\"videoId\":\"([a-zA-Z0-9_-]{11})\")");
        std::smatch match;
        std::string::const_iterator searchStart(responseBody.cbegin());

        int count = 0;
        while (std::regex_search(searchStart, responseBody.cend(), match, videoRegex)) {
            if (count >= limit) break;

            SearchResult result;
            result.id = match[1];
            result.url = "https://www.youtube.com/watch?v=" + result.id;
            
            results.push_back(result);
            searchStart = match.suffix().first;
            count++;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching YouTube results: " << e.what() << std::endl;
    }

    return results;
}

// Returns the first YouTube ID matching the query
std::string YouTubeSearch::GetYouTubeId(const std::string& query) {
    auto results = ytSearch(query, 1);
    return results.empty() ? "" : results[0].id;
}

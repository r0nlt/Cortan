#pragma once

#include <string>
#include <future>
#include <utility>
#include <memory>

namespace cortan::network {

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    std::future<std::pair<bool, std::string>> get(const std::string& url);
    std::future<std::pair<bool, std::string>> post(const std::string& url, const std::string& data);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace cortan::network

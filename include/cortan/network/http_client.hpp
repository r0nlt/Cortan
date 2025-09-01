#pragma once

#include <string>
#include <future>
#include <utility>
#include <memory>
#include <chrono>

namespace cortan::network {

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    // Methods with default timeout (30 seconds)
    std::future<std::pair<bool, std::string>> get(const std::string& url);
    std::future<std::pair<bool, std::string>> post(const std::string& url, const std::string& data);

    // Methods with configurable timeout
    std::future<std::pair<bool, std::string>> get(const std::string& url,
                                                  std::chrono::steady_clock::duration timeout);
    std::future<std::pair<bool, std::string>> post(const std::string& url,
                                                   const std::string& data,
                                                   std::chrono::steady_clock::duration timeout);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace cortan::network

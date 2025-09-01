#include <cortan/network/http_client.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <openssl/ssl.h>
#include <iostream>
#include <string>
#include <chrono>

namespace cortan::network {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class HttpClient::Impl {
public:
    Impl() : ssl_ctx_(ssl::context::tlsv12_client) {
        // Configure SSL context
        ssl_ctx_.set_verify_mode(ssl::verify_peer);
        ssl_ctx_.set_default_verify_paths();
    }

    ~Impl() = default;

    std::pair<bool, std::string> make_request(const std::string& url,
                                             const std::string& method = "GET",
                                             const std::string& data = "",
                                             std::chrono::steady_clock::duration timeout = std::chrono::seconds(30)) {
        try {
            // Parse URL
            std::string host, port, target;
            bool is_https = parse_url(url, host, port, target);

            // Create per-request io_context to ensure thread safety
            net::io_context ioc;

            // Set up deadline timer for timeout
            net::steady_timer deadline_timer{ioc};
            deadline_timer.expires_after(timeout);
            bool timed_out = false;

            deadline_timer.async_wait([&timed_out](const boost::system::error_code& ec) {
                if (!ec) {
                    timed_out = true;
                }
            });

            // Process the request using appropriate helper
            std::pair<bool, std::string> result;
            if (is_https) {
                result = process_https_request(ioc, host, port, target, method, data, deadline_timer);
            } else {
                result = process_http_request(ioc, host, port, target, method, data, deadline_timer);
            }

            // Run the io_context to process all operations
            ioc.run();

            // Check if we timed out
            if (timed_out) {
                return {false, "Request timed out after " +
                       std::to_string(std::chrono::duration_cast<std::chrono::seconds>(timeout).count()) +
                       " seconds"};
            }

            return result;

        } catch (const std::exception& e) {
            return {false, std::string("Network error: ") + e.what()};
        }
    }

private:
    struct UrlComponents {
        std::string scheme;
        std::string userinfo;
        std::string host;
        std::string port;
        std::string path;
        std::string query;
        std::string fragment;
        bool is_https;
    };

    // Enhanced URL parser that handles query strings, fragments, IPv6, and userinfo
    bool parse_url(const std::string& url, std::string& host, std::string& port, std::string& target) {
        UrlComponents components;
        if (!parse_url_components(url, components)) {
            return false;
        }

        host = components.host;
        port = components.port;
        // Compose target as path?query#fragment
        target = components.path;
        if (!components.query.empty()) {
            target += "?" + components.query;
        }
        if (!components.fragment.empty()) {
            target += "#" + components.fragment;
        }

        return components.is_https;
    }

    bool parse_url_components(const std::string& url, UrlComponents& components) {
        std::string url_copy = url;

        // Parse scheme
        size_t scheme_end = url_copy.find("://");
        if (scheme_end == std::string::npos) {
            return false; // Invalid URL format
        }

        components.scheme = url_copy.substr(0, scheme_end);
        components.is_https = (components.scheme == "https");

        if (components.scheme != "http" && components.scheme != "https") {
            return false; // Unsupported scheme
        }

        url_copy = url_copy.substr(scheme_end + 3);

        // Parse userinfo (if present)
        size_t at_pos = url_copy.find('@');
        if (at_pos != std::string::npos) {
            components.userinfo = url_copy.substr(0, at_pos);
            url_copy = url_copy.substr(at_pos + 1);
        }

        // Parse host and port
        size_t path_start = url_copy.find('/');
        if (path_start == std::string::npos) {
            path_start = url_copy.length();
        }

        std::string host_port = url_copy.substr(0, path_start);

        // Handle IPv6 addresses in brackets
        if (!host_port.empty() && host_port[0] == '[') {
            size_t bracket_end = host_port.find(']');
            if (bracket_end == std::string::npos) {
                return false; // Malformed IPv6
            }
            components.host = host_port.substr(1, bracket_end - 1);

            // Check for port after IPv6
            if (bracket_end + 1 < host_port.length() && host_port[bracket_end + 1] == ':') {
                components.port = host_port.substr(bracket_end + 2);
            }
        } else {
            // Regular hostname or IPv4
            size_t colon_pos = host_port.find(':');
            if (colon_pos != std::string::npos) {
                components.host = host_port.substr(0, colon_pos);
                components.port = host_port.substr(colon_pos + 1);
            } else {
                components.host = host_port;
            }
        }

        // Set default port if not specified
        if (components.port.empty()) {
            components.port = components.is_https ? "443" : "80";
        }

        // Parse path, query, and fragment
        if (path_start < url_copy.length()) {
            std::string path_query_fragment = url_copy.substr(path_start);

            // Parse fragment
            size_t fragment_pos = path_query_fragment.find('#');
            if (fragment_pos != std::string::npos) {
                components.fragment = path_query_fragment.substr(fragment_pos + 1);
                path_query_fragment = path_query_fragment.substr(0, fragment_pos);
            }

            // Parse query
            size_t query_pos = path_query_fragment.find('?');
            if (query_pos != std::string::npos) {
                components.query = path_query_fragment.substr(query_pos + 1);
                components.path = path_query_fragment.substr(0, query_pos);
            } else {
                components.path = path_query_fragment;
            }
        } else {
            components.path = "/";
        }

        // Basic validation
        if (components.host.empty()) {
            return false;
        }

        return true;
    }

    ssl::context ssl_ctx_;

    // Helper function to build HTTP request
    template<typename BodyType>
    http::request<BodyType> build_request(const std::string& method,
                                         const std::string& target,
                                         const std::string& host,
                                         const std::string& data = "") {
        http::request<BodyType> req{method == "POST" ? http::verb::post : http::verb::get, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::accept, "*/*");

        if (method == "POST" && !data.empty()) {
            req.set(http::field::content_type, "application/json");
            req.body() = data;
            req.prepare_payload();
        }

        return req;
    }

    // Helper function to handle response
    std::pair<bool, std::string> handle_response(const http::response<http::dynamic_body>& res,
                                                const beast::error_code& shutdown_ec = {}) {
        if (res.result() == http::status::ok) {
            return {true, std::string(beast::buffers_to_string(res.body().data()))};
        } else {
            std::string error_msg = "HTTP " + std::to_string(static_cast<int>(res.result())) + " " +
                                    std::string(res.reason());
            if (shutdown_ec) {
                error_msg += " (Connection error: " + shutdown_ec.message() + ")";
            }
            return {false, error_msg};
        }
    }

    // Helper function for HTTPS request processing
    std::pair<bool, std::string> process_https_request(net::io_context& ioc,
                                                      const std::string& host,
                                                      const std::string& port,
                                                      const std::string& target,
                                                      const std::string& method,
                                                      const std::string& data,
                                                      net::steady_timer& deadline_timer) {
        try {
            // Create SSL stream
            ssl::stream<tcp::socket> stream{ioc, ssl_ctx_};

            // Look up the domain name
            tcp::resolver resolver{ioc};
            auto const results = resolver.resolve(host, port);

            // Make the connection
            net::connect(stream.next_layer(), results.begin(), results.end());

            // Set SNI hostname
            if(!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
                return {false, "Failed to set SNI hostname"};
            }

            // Perform the SSL handshake
            stream.handshake(ssl::stream_base::client);

            // Send the HTTP request
            auto req = build_request<http::string_body>(method, target, host, data);
            http::write(stream, req);

            // Receive the HTTP response
            beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(stream, buffer, res);

            // Cancel the deadline timer since we completed successfully
            deadline_timer.cancel();

            // Close the connection
            beast::error_code ec;
            stream.shutdown(ec);

            if (ec) {
                std::cerr << "SSL shutdown error: " << ec.message() << std::endl;
            }

            return handle_response(res, ec);

        } catch (const std::exception& e) {
            return {false, std::string("HTTPS request error: ") + e.what()};
        }
    }

    // Helper function for HTTP request processing
    std::pair<bool, std::string> process_http_request(net::io_context& ioc,
                                                     const std::string& host,
                                                     const std::string& port,
                                                     const std::string& target,
                                                     const std::string& method,
                                                     const std::string& data,
                                                     net::steady_timer& deadline_timer) {
        try {
            // Create socket
            tcp::socket socket{ioc};

            // Look up the domain name
            tcp::resolver resolver{ioc};
            auto const results = resolver.resolve(host, port);

            // Make the connection
            net::connect(socket, results.begin(), results.end());

            // Send the HTTP request
            auto req = build_request<http::string_body>(method, target, host, data);
            http::write(socket, req);

            // Receive the HTTP response
            beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(socket, buffer, res);

            // Cancel the deadline timer since we completed successfully
            deadline_timer.cancel();

            // Close the connection
            beast::error_code ec;
            socket.shutdown(tcp::socket::shutdown_both, ec);

            return handle_response(res);

        } catch (const std::exception& e) {
            return {false, std::string("HTTP request error: ") + e.what()};
        }
    }
};

HttpClient::HttpClient() : impl_(std::make_unique<Impl>()) {}

HttpClient::~HttpClient() = default;

std::future<std::pair<bool, std::string>> HttpClient::get(const std::string& url) {
    return std::async(std::launch::async, [this, url]() -> std::pair<bool, std::string> {
        return impl_->make_request(url, "GET", "", std::chrono::seconds(30));
    });
}

std::future<std::pair<bool, std::string>> HttpClient::post(const std::string& url, const std::string& data) {
    return std::async(std::launch::async, [this, url, data]() -> std::pair<bool, std::string> {
        return impl_->make_request(url, "POST", data, std::chrono::seconds(30));
    });
}

std::future<std::pair<bool, std::string>> HttpClient::get(const std::string& url,
                                                          std::chrono::steady_clock::duration timeout) {
    return std::async(std::launch::async, [this, url, timeout]() -> std::pair<bool, std::string> {
        return impl_->make_request(url, "GET", "", timeout);
    });
}

std::future<std::pair<bool, std::string>> HttpClient::post(const std::string& url,
                                                           const std::string& data,
                                                           std::chrono::steady_clock::duration timeout) {
    return std::async(std::launch::async, [this, url, data, timeout]() -> std::pair<bool, std::string> {
        return impl_->make_request(url, "POST", data, timeout);
    });
}

} // namespace cortan::network

#include <cortan/network/http_client.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>
#include <string>

namespace cortan::network {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class HttpClient::Impl {
public:
    Impl() : ioc_(), ssl_ctx_(ssl::context::tlsv12_client) {
        // Configure SSL context
        ssl_ctx_.set_verify_mode(ssl::verify_peer);
        ssl_ctx_.set_default_verify_paths();
    }

    ~Impl() = default;

    std::pair<bool, std::string> make_request(const std::string& url,
                                             const std::string& method = "GET",
                                             const std::string& data = "") {
        try {
            // Parse URL
            std::string host, port, target;
            bool is_https = parse_url(url, host, port, target);

            // Create resolver and socket
            tcp::resolver resolver{ioc_};

            if (is_https) {
                // HTTPS request
                ssl::stream<tcp::socket> stream{ioc_, ssl_ctx_};

                // Look up the domain name
                auto const results = resolver.resolve(host, port);

                // Make the connection on the IP address we get from a lookup
                net::connect(stream.next_layer(), results.begin(), results.end());

                // Perform the SSL handshake
                stream.handshake(ssl::stream_base::client);

                // Send the HTTP request to the remote host
                http::request<http::string_body> req{method == "POST" ? http::verb::post : http::verb::get, target, 11};
                req.set(http::field::host, host);
                req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
                req.set(http::field::accept, "*/*");

                if (method == "POST" && !data.empty()) {
                    req.set(http::field::content_type, "application/json");
                    req.body() = data;
                    req.prepare_payload();
                }

                http::write(stream, req);

                // Receive the HTTP response
                beast::flat_buffer buffer;
                http::response<http::dynamic_body> res;
                http::read(stream, buffer, res);

                // Close the connection
                beast::error_code ec;
                stream.shutdown(ec);

                if (res.result() == http::status::ok) {
                    return {true, std::string(beast::buffers_to_string(res.body().data()))};
                } else {
                    return {false, "HTTP " + std::to_string(static_cast<int>(res.result())) + " " +
                           std::string(res.reason())};
                }
            } else {
                // HTTP request (non-HTTPS)
                tcp::socket socket{ioc_};

                // Look up the domain name
                auto const results = resolver.resolve(host, port);

                // Make the connection on the IP address we get from a lookup
                net::connect(socket, results.begin(), results.end());

                // Send the HTTP request to the remote host
                http::request<http::string_body> req{method == "POST" ? http::verb::post : http::verb::get, target, 11};
                req.set(http::field::host, host);
                req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
                req.set(http::field::accept, "*/*");

                if (method == "POST" && !data.empty()) {
                    req.set(http::field::content_type, "application/json");
                    req.body() = data;
                    req.prepare_payload();
                }

                http::write(socket, req);

                // Receive the HTTP response
                beast::flat_buffer buffer;
                http::response<http::dynamic_body> res;
                http::read(socket, buffer, res);

                // Close the connection
                beast::error_code ec;
                socket.shutdown(tcp::socket::shutdown_both, ec);

                if (res.result() == http::status::ok) {
                    return {true, std::string(beast::buffers_to_string(res.body().data()))};
                } else {
                    return {false, "HTTP " + std::to_string(static_cast<int>(res.result())) + " " +
                           std::string(res.reason())};
                }
            }
        } catch (const std::exception& e) {
            return {false, std::string("Network error: ") + e.what()};
        }
    }

private:
    bool parse_url(const std::string& url, std::string& host, std::string& port, std::string& target) {
        std::string url_copy = url;

        // Check if HTTPS
        bool is_https = (url_copy.substr(0, 8) == "https://");
        if (is_https) {
            url_copy = url_copy.substr(8);
        } else if (url_copy.substr(0, 7) == "http://") {
            url_copy = url_copy.substr(7);
        }

        // Find host and target
        size_t slash_pos = url_copy.find('/');
        if (slash_pos != std::string::npos) {
            host = url_copy.substr(0, slash_pos);
            target = url_copy.substr(slash_pos);
        } else {
            host = url_copy;
            target = "/";
        }

        // Extract port if specified
        size_t colon_pos = host.find(':');
        if (colon_pos != std::string::npos) {
            port = host.substr(colon_pos + 1);
            host = host.substr(0, colon_pos);
        } else {
            port = is_https ? "443" : "80";
        }

        return is_https;
    }

    net::io_context ioc_;
    ssl::context ssl_ctx_;
};

HttpClient::HttpClient() : impl_(std::make_unique<Impl>()) {}

HttpClient::~HttpClient() = default;

std::future<std::pair<bool, std::string>> HttpClient::get(const std::string& url) {
    return std::async(std::launch::async, [this, url]() -> std::pair<bool, std::string> {
        return impl_->make_request(url, "GET");
    });
}

std::future<std::pair<bool, std::string>> HttpClient::post(const std::string& url, const std::string& data) {
    return std::async(std::launch::async, [this, url, data]() -> std::pair<bool, std::string> {
        return impl_->make_request(url, "POST", data);
    });
}

} // namespace cortan::network

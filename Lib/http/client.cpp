#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <Windows.h>
#endif

using namespace boost::asio;
using namespace boost::asio::ip;

void send_http_request(const std::string& host, const std::string& port, const std::string& path) {
    io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    connect(socket, endpoint_iterator);

    // * Create HTTP GET request
    std::string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n\r\n";

    // * Send the request
    boost::asio::write(socket, buffer(request));

    // * Receive the response
    boost::system::error_code error;
    while (true) {
        char buf[512];
        size_t len = socket.read_some(buffer(buf), error);

        if (error == boost::asio::error::eof)
            break;  // * Connection closed by peer
        else if (error)
            throw boost::system::system_error(error);  // * Some other error

        std::cout.write(buf, len);
    }
}

void post_http_request(const std::string& host, const std::string& port, const std::string& path, const std::string& data) {
    io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    connect(socket, endpoint_iterator);

    // * Create HTTP POST request
    std::string request = "POST " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + std::to_string(data.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += data;

    // * Send the request
    boost::asio::write(socket, buffer(request));

    // * Receive the response
    boost::system::error_code error;
    std::string response;
    while (true) {
        char buf[512];
        size_t len = socket.read_some(buffer(buf), error);

        if (error == boost::asio::error::eof)
            break;  // * Connection closed by peer
        else if (error)
            throw boost::system::system_error(error);  // * Some other error

        response.append(buf, len);
    }

    // * Find the end of the headers
    size_t header_end = response.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        std::cout << response.substr(header_end + 4) << std::endl;
    }
}

#include <omp.h>

int main() {
    // * * send to 127.0.0.1:3000 with path /json
    send_http_request("127.0.0.1", "3000", "/json");
    #pragma omp parallel
    {
        while (true) {
            post_http_request("127.0.0.1", "3000", "/message", "{\"message\": \"admin\", \"password\": \"admin\"}");
        }
    }

    return 0;
}

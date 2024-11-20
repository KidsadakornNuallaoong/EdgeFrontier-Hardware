// server.cpp
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <memory>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace websocket = beast::websocket; 
namespace net = boost::asio;             
using tcp = boost::asio::ip::tcp;       

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    explicit WebSocketSession(tcp::socket socket) 
        : ws(std::move(socket)) {}

    void run() {
        ws.async_accept(
            beast::bind_front_handler(&WebSocketSession::on_accept, shared_from_this()));
    }

private:
    websocket::stream<tcp::socket> ws;
    beast::flat_buffer buffer; // Buffer for received data

    void on_accept(beast::error_code ec) {
        if (ec) return fail(ec, "accept");
        do_read();
    }

    void do_read() {
        ws.async_read(buffer,
            beast::bind_front_handler(&WebSocketSession::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) return fail(ec, "read");

        // Handle the incoming message
        std::string message = beast::buffers_to_string(buffer.data());
        buffer.clear();
        std::cout << "Received: " << message << std::endl;

        // Echo back the message
        ws.async_write(net::buffer(message),
            beast::bind_front_handler(&WebSocketSession::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) return fail(ec, "write");
        do_read(); // Continue reading
    }

    static void fail(beast::error_code ec, const char* what) {
        std::cerr << what << ": " << ec.message() << std::endl;
    }
};

class WebSocketServer {
public:
    WebSocketServer(net::io_context& ioc, tcp::endpoint endpoint) 
        : acceptor(ioc, std::move(endpoint)) {
        do_accept();
    }

private:
    tcp::acceptor acceptor;

    void do_accept() {
        acceptor.async_accept(
            beast::bind_front_handler(&WebSocketServer::on_accept, this));
    }

    void on_accept(beast::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<WebSocketSession>(std::move(socket))->run();
        }
        do_accept(); // Accept the next connection
    }
};

int main() {
    try {
        net::io_context ioc;
        tcp::endpoint endpoint(tcp::v4(), 8080);
        WebSocketServer server(ioc, endpoint);
        ioc.run();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

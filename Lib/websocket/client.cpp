#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/common/thread.hpp>
#include <iostream>
#include <nlohmann/json.hpp>  // Make sure to install the JSON library if using this.

typedef websocketpp::client<websocketpp::config::asio_client> client;

void on_open(client* c, websocketpp::connection_hdl hdl) {
    // Create a JSON object
    nlohmann::json j;
    j["message"] = "Hello, WebSocket server!";
    j["number"] = 42;

    // Serialize the JSON object
    std::string message = j.dump();

    // Send the message to the server
    c->send(hdl, message, websocketpp::frame::opcode::text);
}

int main() {
    client c;

    try {
        // Set logging settings
        c.set_error_channels(websocketpp::log::elevel::none);
        c.set_access_channels(websocketpp::log::alevel::none);

        // Initialize Asio
        c.init_asio();

        // Set open handler
        c.set_open_handler(websocketpp::lib::bind(&on_open, &c, websocketpp::lib::placeholders::_1));

        // Create a connection to the WebSocket server
        std::string uri = "ws://localhost:8181";
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);

        if (ec) {
            std::cout << "Error: " << ec.message() << std::endl;
            return 1;
        }

        // Connect to server
        c.connect(con);

        // Start the ASIO io_service
        c.run();
    } catch (const websocketpp::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}
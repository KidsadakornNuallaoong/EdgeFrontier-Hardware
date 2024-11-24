#if !defined(WEBSOCKET_HELPER_HPP)
#define WEBSOCKET_HELPER_HPP

#include <iostream>
#include <string>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <nlohmann/json.hpp>
#include <asio.hpp>

using json = nlohmann::json;
using websocketpp::connection_hdl;
using websocketpp::client;
using websocketpp::config::asio_client;
using websocketpp::lib::asio::ssl::context;
using websocketpp::lib::asio::ssl::verify_none;

// Define the WebSocket client for secure connection
typedef websocketpp::config::asio_tls_client asio_tls_client;
typedef websocketpp::client<asio_tls_client> tls_client;

// Define the WebSocket client for non-secure connection
typedef websocketpp::client<asio_client> client;

class WebSocketClient {
    private:
        // * wss:// WebSocket client
        tls_client m_client;
        // * ws:// WebSocket client
        client m_client_no_secure;
        // * Data to be sent
        std::string m_data_to_send;
        bool is_secure = true;
    
    public:
        WebSocketClient() {
            // * Initialize WebSocket client
            m_client.init_asio();
            m_client_no_secure.init_asio();
            
            // * SSL context setup for secure WebSocket (wss://)
            m_client.set_tls_init_handler([this](connection_hdl) {
                std::shared_ptr<context> ctx = std::make_shared<context>(context::sslv23);
                ctx->set_verify_mode(verify_none); // * Disable SSL verification (can be customized)
                return ctx;
            });
            
            // * Set handlers for secure WebSocket
            m_client.set_open_handler(std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
            m_client.set_message_handler(std::bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
            m_client.set_close_handler(std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
            
            // * Set handlers for non-secure WebSocket
            m_client_no_secure.set_open_handler(std::bind(&WebSocketClient::on_open_no_secure, this, std::placeholders::_1));
            m_client_no_secure.set_message_handler(std::bind(&WebSocketClient::on_message_no_secure, this, std::placeholders::_1, std::placeholders::_2));
            m_client_no_secure.set_close_handler(std::bind(&WebSocketClient::on_close_no_secure, this, std::placeholders::_1));
        }
        
        void send_json(const std::string& uri, const json& data) {
            try {
                websocketpp::lib::error_code ec;
                
                // * Create the connection
                if (is_secure_uri(uri)) {
                    tls_client::connection_ptr con = m_client.get_connection(uri, ec);
                    if (ec) {
                        std::cerr << "Error creating connection: " << ec.message() << std::endl;
                        return;
                    }
                    // * Set the data to be sent once the connection is open
                    m_data_to_send = data.dump();
                    // * Connect to the WebSocket server
                    m_client.connect(con);
                    m_client.run();  // * This will start the WebSocket client event loop
                } else {
                    client::connection_ptr con = m_client_no_secure.get_connection(uri, ec);
                    if (ec) {
                        std::cerr << "Error creating connection: " << ec.message() << std::endl;
                        return;
                    }
                    // * Set the data to be sent once the connection is open
                    m_data_to_send = data.dump();
                    // * Connect to the WebSocket server
                    m_client_no_secure.connect(con);
                    m_client_no_secure.run();  // * This will start the WebSocket client event loop
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception: " << e.what() << std::endl;
            }
        }

        bool is_secure_uri(const std::string& uri) {
            return uri.substr(0, 3) == "wss";
        }

        void on_open(connection_hdl hdl) {
            m_client.send(hdl, m_data_to_send, websocketpp::frame::opcode::text);
        }

        void on_message(connection_hdl hdl, tls_client::message_ptr msg) {
            std::cout << "Received message: " << msg->get_payload() << std::endl;
        }

        void on_close(connection_hdl hdl) {
            std::cout << "Connection closed." << std::endl;
        }

        void on_open_no_secure(connection_hdl hdl) {
            m_client_no_secure.send(hdl, m_data_to_send, websocketpp::frame::opcode::text);
        }


        void on_message_no_secure(connection_hdl hdl, client::message_ptr msg) {
            std::cout << "Received message: " << msg->get_payload() << std::endl;
        }

        void on_close_no_secure(connection_hdl hdl) {
            std::cout << "Connection closed." << std::endl;
        }
                    //
};

#endif // WEBSOCKET_HELPER_HPP)

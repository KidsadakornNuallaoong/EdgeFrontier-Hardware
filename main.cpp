/**
 * @file main.cpp
 * @brief WebSocket client for sending simulated sensor data.
 * 
 * This file contains the implementation of a WebSocket client that sends simulated sensor data
 * to a WebSocket server. The sensor data is generated randomly and updated periodically.
 * The client supports both secure (wss) and non-secure (ws) WebSocket connections.
 * 
 * The main components of this implementation include:
 * - Sensor data simulation and updating.
 * - WebSocket client setup and connection handling.
 * - Environment variable loading from a .env file.
 * 
 * Dependencies:
 * - nlohmann/json for JSON handling.
 * - websocketpp for WebSocket client functionality.
 * - Boost.Asio for SSL/TLS support.
 * 
 * @note This implementation uses multiple threads for updating sensor data, printing sensor data,
 *       and handling WebSocket communication.
 * 
 * @section author Author
 * Kidsadakorn Nuallaoong
 * @date 2023-10-05
 */
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio.hpp>
#include <boost/asio/ssl/context.hpp>

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;

// * Global mutex for thread safety
std::mutex mtx;

// * Random number generator for sensor data simulation
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 100.0);

const char* Event[] = {"Cold", "Warm", "Hot", "Dry", "Wet", "Normal", "Unknown"};
const char* HardwareID[] = {"EF-001", "EF-002"};

// * JSON structure holding initial sensor data
nlohmann::json sensor_data = {
    {"TimeStamp", "18/11/24 08:06:59"},
    {"HardwareID", "EF-001"},
    {"Event", "Cold"},
    {"Data", {
        {"CO2", dis(gen)},
        {"VOC", dis(gen)},
        {"RA", dis(gen)},
        {"TEMP", dis(gen)},
        {"HUMID", dis(gen)},
        {"PRESSURE", dis(gen)}
    }}
};

// * Function to update sensor data with new random values
void update_sensor_data(nlohmann::json& j) {
    std::lock_guard<std::mutex> lock(mtx);
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%d/%m/%y %H:%M:%S");
    j["TimeStamp"] = ss.str();
    std::uniform_int_distribution<> hardware_dist(0, sizeof(HardwareID)/sizeof(HardwareID[0]) - 1);
    std::uniform_int_distribution<> event_dist(0, sizeof(Event)/sizeof(Event[0]) - 1);
    j["HardwareID"] = HardwareID[hardware_dist(gen)];
    j["Event"] = Event[event_dist(gen)];
    j["Data"]["CO2"] = dis(gen);
    j["Data"]["VOC"] = dis(gen);
    j["Data"]["RA"] = dis(gen);
    j["Data"]["TEMP"] = dis(gen);
    j["Data"]["HUMID"] = dis(gen);
    j["Data"]["PRESSURE"] = dis(gen);
}

/**
 * @brief Prints the sensor data JSON to the console.
 * 
 * This function prints the sensor data JSON to the console.
 * It acquires a lock on the mutex to ensure thread safety.
 * 
 * @note This function is intended to be run in a separate thread.
 */
void print_json() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << sensor_data.dump(4) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200000));
    }
}

/**
 * @brief Updates the sensor data JSON with new random values.
 * 
 * This function updates the sensor data JSON with new random values.
 * It acquires a lock on the mutex to ensure thread safety.
 * 
 * @note This function is intended to be run in a separate thread.
 */
void update_json_loop() {
    while (true) {
        update_sensor_data(sensor_data);
        std::this_thread::sleep_for(std::chrono::microseconds(200000));
    }
}

/**
 * @brief Sends the sensor data JSON to the WebSocket server.
 * 
 * This function sends the sensor data JSON to the WebSocket server.
 * It acquires a lock on the mutex to ensure thread safety.
 * 
 * @param c A pointer to the WebSocket client.
 * @param hdl The WebSocket connection handle.
 * 
 * @note This function is intended to be run in a separate thread.
 */
void send_json_loop(client* c, websocketpp::connection_hdl hdl) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx); // * Ensuring thread safety
            std::string message = sensor_data.dump(); // * Serialize the JSON message
            c->send(hdl, message, websocketpp::frame::opcode::text); // * Send the JSON message to the server
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200000)); // * Sleep for 1 second before sending again
    }
}

/**
 * @brief Sends the sensor data JSON to the WebSocket server.
 *
 * This function sends the sensor data JSON to the WebSocket server.
 * It acquires a lock on the mutex to ensure thread safety.
 *
 * @param c A pointer to the TLS WebSocket client.
 * @param hdl The WebSocket connection handle.
 *
 * @note This function is intended to be run in a separate thread.
 */
void send_json_loop_secure(tls_client* c, websocketpp::connection_hdl hdl) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::string message = sensor_data.dump();
            websocketpp::lib::error_code ec;
            c->send(hdl, message, websocketpp::frame::opcode::text, ec);
            if (ec) {
                std::cerr << "Send error: " << ec.message() << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200000));
    }
}

/**
 * @brief Handles the on_open event for a non-secure WebSocket connection.
 * 
 * This function starts a separate thread for sending the sensor data JSON to the WebSocket server.
 * 
 * @param c A pointer to the WebSocket client.
 * @param hdl The WebSocket connection handle.
 */
void on_open(client* c, websocketpp::connection_hdl hdl) {
    std::thread send_thread(send_json_loop, c, hdl);
    send_thread.detach();
}

/**
 * @brief Handles the on_open event for a secure WebSocket connection.
 *
 * This function starts a separate thread for sending the sensor data JSON to the WebSocket server.
 *
 * @param c A pointer to the TLS WebSocket client.
 * @param hdl The WebSocket connection handle.
 */
void on_open_secure(tls_client* c, websocketpp::connection_hdl hdl) {
    std::thread send_thread(send_json_loop_secure, c, hdl);
    send_thread.detach();
}

/**
 * @brief Initializes the TLS context for the WebSocket connection.
 *
 * This function initializes the TLS context for the WebSocket connection.
 * It sets the TLS options for the context to use TLSv1.2 and disable SSLv2 and SSLv3.
 *
 * @param hdl The WebSocket connection handle.
 * @return A shared pointer to the TLS context.
 *
 * @throws std::exception If an error occurs during TLS context initialization.
 */
std::shared_ptr<boost::asio::ssl::context> on_tls_init(websocketpp::connection_hdl) {
    std::shared_ptr<boost::asio::ssl::context> ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv12_client));
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cerr << "TLS error: " << e.what() << std::endl;
        throw;
    }
    return ctx;
}

/**
 * @brief Handles a non-secure WebSocket connection to the specified URI.
 *
 * This function sets up a WebSocket client, connects to the specified URI,
 * and starts separate threads for handling the WebSocket connection, updating
 * sensor data, and printing sensor data.
 *
 * @param uri The URI of the WebSocket server to connect to.
 * @param c A pointer to the WebSocket client.
 *
 * @throws websocketpp::exception If a WebSocket error occurs.
 * @throws std::exception If a general error occurs.
 */
void handle_no_secure(const std::string &uri, client *c)
{
    try
    {
        // * Set logging settings for WebSocket client
        c->set_error_channels(websocketpp::log::elevel::none);
        c->set_access_channels(websocketpp::log::alevel::none);
        // * Initialize ASIO for WebSocket client
        c->init_asio();
        // * Set the open handler for the WebSocket connection
        c->set_open_handler(websocketpp::lib::bind(&on_open, c, websocketpp::lib::placeholders::_1));
        // * Create connection to WebSocket server
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c->get_connection(uri, ec);
        if (ec) {
            std::cerr << "Error: " << ec.message() << std::endl;
            return;
        }
        // * Connect to the server
        c->connect(con);
        // * Start a separate thread for WebSocket client to handle the connection
        std::thread websocket_thread([c]() {
            c->run(); // * Run the WebSocket client
        });
        // * Start threads for updating and printing the sensor data
        std::thread update_thread(update_json_loop);
        std::thread print_thread(print_json);
        // * Wait for threads to finish
        update_thread.join();
        print_thread.join();
        websocket_thread.join();
    }
    catch (const websocketpp::exception &e)
    {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Handles a secure WebSocket connection.
 *
 * This function sets up and manages a secure WebSocket connection using the provided URI and TLS client.
 * It initializes the WebSocket client, sets up handlers, and starts threads for handling the connection
 * and updating/printing sensor data.
 *
 * @param uri The URI of the WebSocket server to connect to.
 * @param tc A pointer to the TLS client used for the WebSocket connection.
 *
 * @note This function starts three separate threads:
 *       - One for running the WebSocket client.
 *       - One for updating sensor data.
 *       - One for printing sensor data.
 *
 * @throws websocketpp::exception If a WebSocket error occurs.
 * @throws std::exception If a general error occurs.
 */
void handle_secure(const std::string &uri, tls_client *tc)
{
    try
    {
        // * Set logging settings for WebSocket client
        tc->set_error_channels(websocketpp::log::elevel::none);
        tc->set_access_channels(websocketpp::log::alevel::none);
        // * Initialize ASIO for WebSocket client
        tc->init_asio();
        // * Set the open handler for the WebSocket connection
        tc->set_open_handler(websocketpp::lib::bind(&on_open_secure, tc, websocketpp::lib::placeholders::_1));
        // * Set the TLS initialization handler for the WebSocket connection
        tc->set_tls_init_handler(websocketpp::lib::bind(&on_tls_init, websocketpp::lib::placeholders::_1));
        // * Create connection to WebSocket server
        websocketpp::lib::error_code ec;
        tls_client::connection_ptr con = tc->get_connection(uri, ec);
        if (ec)
        {
            std::cerr << "Error: " << ec.message() << std::endl;
            return;
        }
        // * Connect to the server
        tc->connect(con);
        // * Start a separate thread for WebSocket client to handle the connection
        std::thread websocket_thread([tc]()
                                        {
                                            tc->run(); // * Run the WebSocket client
                                        });
        // * Start threads for updating and printing the sensor data
        std::thread update_thread(update_json_loop);
        std::thread print_thread(print_json);
        // * Wait for threads to finish
        update_thread.join();
        print_thread.join();
        websocket_thread.join();
    }
    catch (const websocketpp::exception &e)
    {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Loads environment variables from a .env file into an unordered_map.
 *
 * This function reads a .env file specified by the filePath and loads the key-value pairs
 * into an unordered_map. It also optionally sets the variables in the environment.
 *
 * @param filePath The path to the .env file.
 * @return An unordered_map containing the key-value pairs from the .env file.
 *
 * @note Lines that are empty or start with '#' are ignored.
 * @note Lines that do not contain an '=' delimiter are considered malformed and are ignored.
 * @note Whitespace trimming around keys and values is not implemented but can be added if needed.
 * @note On Windows, environment variables are set using _putenv. On other platforms, setenv is used.
 */
std::unordered_map<std::string, std::string> loadEnvFile(const std::string& filePath) {
    std::unordered_map<std::string, std::string> envMap;
    std::ifstream envFile(filePath);
    if (!envFile.is_open()) {
        std::cerr << "Error: Unable to open .env file: " << filePath << std::endl;
        return envMap;
    }

    std::string line;
    while (std::getline(envFile, line)) {
        // * Skip comments or empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) {
            std::cerr << "Error: Malformed line in .env file: " << line << std::endl;
            continue;
        }

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        // * Trim whitespace (optional, implement trim logic if needed)
        envMap[key] = value;

        // * Optionally set the variable in the environment
        #ifdef _WIN32
        _putenv((key + "=" + value).c_str());
        #else
        setenv(key.c_str(), value.c_str(), 1);
        #endif
    }

    return envMap;
}

/**
 * @brief Checks if the URI is secure (wss://).
 * 
 * @param uri The URI to check.
 * @return true if the URI is secure (wss://), false otherwise.
 * 
 * @note This function checks if the URI starts with "wss://".
 * 
 * @note This function is case-sensitive.
 * **/
bool is_secure(const std::string &uri)
{
    return uri.substr(0, 3) == "wss"; // * * if detected wss:// * return true
}

int main() {
    // * Load environment variables from .env file
    std::string envFilePath = "example.env";
    auto envMap = loadEnvFile(envFilePath);

    // * WebSocket client initialization
    const char* uri_cstr = getenv("WS_URI");
    if (uri_cstr == nullptr) {
        std::cerr << "Error: WS_URI environment variable is not set." << std::endl;
        return 1;
    }
    std::string uri(uri_cstr);
    std::cout << "Connecting to WebSocket server at: " << uri << std::endl;
    client c;
    tls_client tc;

    is_secure(uri) ? handle_secure(uri, &tc) : handle_no_secure(uri, &c);
    return 0;
}
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
 * @date 2024-11-05
 **/
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
#include <iomanip>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio.hpp>
#include <boost/asio/ssl/context.hpp>
#include "Libs/log_manager.hpp"
#include "Libs/MLP/MLP.hpp"
#include "Libs/http_helper.hpp"

// * Enum for operating mode
enum mode {SAFE_MODE, PREDICTION_MODE};
mode current_mode = SAFE_MODE;

#ifdef _WIN32
    #include <thread>
    #include <conio.h> // For _kbhit and _getch
    
    bool is_run = true;

    void checkInput_main() {
        while (is_run) {
            if (_kbhit()) {
                char ch = _getch();
                if (ch == 't' || ch == 'T') {
                    is_run = false;
                }
            }
        }
    }

#elif __linux__
    #include <thread>
    #include <atomic>
    #include <unistd.h>
    #include <fcntl.h>
    #include <termios.h>

    std::atomic<bool> is_run(true);

    bool kbhit_main() {
        struct termios oldt, newt;
        int ch;
        int oldf;

        // Get the current terminal settings
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        // Disable canonical mode and echo
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        // Set stdin to non-blocking mode
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        // Check for input
        ch = getchar();

        // Restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if(ch != EOF) {
            ungetc(ch, stdin);
            return true;
        }

        return false;
    }

    void checkInput_main() {
        while (is_run) {
            if (kbhit_main()) {
                char ch = getchar();
                if (ch == 't' || ch == 'T') {
                    is_run = false;
                }

                if (ch == 'm' || ch == 'M') {
                    current_mode = (current_mode == SAFE_MODE) ? PREDICTION_MODE : SAFE_MODE;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay to avoid high CPU usage
        }
    }
#endif

typedef websocketpp::client<websocketpp::config::asio_client> client;

typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;

// * Global log manager instance
LogManager& logManager = LogManager::getInstance();

// * Global mutex for thread safety
std::mutex mtx;

// * HTTP client
HTTP http;
std::string rest_main_server_cstr = "http://localhost:8181";

// * Random number generator for sensor data simulation
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 100.0);

// * Event types for sensor data
const char* Event[] = {"Cold", "Warm", "Hot", "Dry", "Wet", "Normal", "Unknown"};
std::string HardwareID = "UNKNOWn";

// * JSON structure holding initial sensor data
nlohmann::json sensor_data = {
    {"TimeStamp", "2023-10-05 12:00:00"},
    {"HardwareID", HardwareID},
    {"Event", "Cold"},
    {"Mode", (current_mode == PREDICTION_MODE) ? "PREDCITION" : "SAFE"},
    {"Data", {
        {"CO2", dis(gen)},
        {"VOC", dis(gen)},
        {"RA", dis(gen)},
        {"TEMP", dis(gen)},
        {"HUMID", dis(gen)},
        {"PRESSURE", dis(gen)}
    }},
    {"Prediction", {
        {"Cold", 0.0},
        {"Warm", 0.0},
        {"Hot", 0.0},
        {"Dry", 0.0},
        {"Wet", 0.0},
        {"Normal", 0.0},
        {"Unknown", 0.0}
    }}
};

enum speed {SLOW, MEDIUM, FAST};
speed current_speed = SLOW;

nlohmann::json info = {
    {"HardwareID", HardwareID},
    {"Mode", (current_mode == PREDICTION_MODE) ? "PREDCITION" : "SAFE"},
    {"Speed", (current_speed == SLOW) ? "SLOW" : (current_speed == MEDIUM) ? "MEDIUM" : "FAST"}
};

void delay(){
    current_speed == SLOW ? std::this_thread::sleep_for(std::chrono::seconds(1)) :
    (current_speed == MEDIUM) ? std::this_thread::sleep_for(std::chrono::microseconds(200000)) :
    std::this_thread::sleep_for(std::chrono::microseconds(100000));
}

void delay_server(){ std::this_thread::sleep_for(std::chrono::seconds(1)); }

// * Function to update sensor data with new Arandom values
void update_sensor_data(nlohmann::json& j) {
    std::lock_guard<std::mutex> lock(mtx);
    // * update timestamp
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    j["TimeStamp"] = ss.str();
    // * update data
    std::uniform_int_distribution<> hardware_dist(0, sizeof(HardwareID)/sizeof(HardwareID[0]) - 1);
    std::uniform_int_distribution<> event_dist(0, sizeof(Event)/sizeof(Event[0]) - 1);
    j["HardwareID"] = HardwareID;
    j["Event"] = Event[event_dist(gen)];
    j["Mode"] = (current_mode == PREDICTION_MODE) ? "PREDICTION" : "SAFE";
    j["Data"]["CO2"] = dis(gen);
    j["Data"]["VOC"] = dis(gen);
    j["Data"]["RA"] = dis(gen);
    j["Data"]["TEMP"] = dis(gen);
    j["Data"]["HUMID"] = dis(gen);
    j["Data"]["PRESSURE"] = dis(gen);

    // Log the updated sensor data
    // logManager.setLogLevel(LogManager::DEBUG);
    // logManager.log(LogManager::DEBUG, "Sensor data updated");
}

void update_info(nlohmann::json& j) {
    std::lock_guard<std::mutex> lock(mtx);
    j["HardwareID"] = HardwareID;
    j["Mode"] = (current_mode == PREDICTION_MODE) ? "PREDICTION" : "SAFE";
    j["Speed"] = (current_speed == SLOW) ? "SLOW" : (current_speed == MEDIUM) ? "MEDIUM" : "FAST";
}

/**
 * @brief Prints the sensor data JSON to the console.
 * 
 * This function prints the sensor data JSON to the console.
 * It acquires a lock on the mutex to ensure thread safety.
 * 
 * @note This function is intended to be run in a separate thread.
 **/
void print_json() {
    std::cout << "Starting print_json thread" << std::endl;
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Starting print json thread");

    while (is_run) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            // * if safe mode dump json but not dump prediction
            if (current_mode == SAFE_MODE) {
                nlohmann::json safe_mode_data = sensor_data;
                safe_mode_data.erase("Prediction");
                std::cout << safe_mode_data.dump(4) << std::endl;
            } else {
                std::cout << sensor_data.dump(4) << std::endl;
            }
        }
        delay();
    }

    std::cout << "Exiting print_json thread" << std::endl;
    logManager.setLogLevel(LogManager::INFO);
    logManager.log(LogManager::INFO, "Exiting print json thread");
}

/**
 * @brief Updates the sensor data JSON with new random values.
 * 
 * This function updates the sensor data JSON with new random values.
 * It acquires a lock on the mutex to ensure thread safety.
 * 
 * @note This function is intended to be run in a separate thread.
 **/
void update_json_loop() {
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Starting update json loop thread");

    while (is_run) {
        update_sensor_data(sensor_data);
        update_info(info);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Exiting update_json_loop thread" << std::endl;
    logManager.setLogLevel(LogManager::INFO);
    logManager.log(LogManager::INFO, "Exiting update json loop thread");
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
 **/
void send_json_loop(client* c, websocketpp::connection_hdl hdl) {
    std::cout << "Starting send_json_loop thread" << std::endl;
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Starting send json loop thread");
    
    while (is_run) {
        {
            std::lock_guard<std::mutex> lock(mtx); // Ensuring thread safety
            std::string message = sensor_data.dump(); // Serialize the JSON message
            c->send(hdl, message, websocketpp::frame::opcode::text); // Send the JSON message to the server
        }
        delay();
    }

    std::cout << "Exiting send_json_loop thread" << std::endl;
    logManager.setLogLevel(LogManager::INFO);
    logManager.log(LogManager::INFO, "Exiting send json loop thread");
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
 **/
void send_json_loop_secure(tls_client* tc, websocketpp::connection_hdl hdl) {
    std::cout << "Starting send_json_loop_secure thread" << std::endl;
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Starting send json loop secure thread");

    while (is_run){
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::string message = sensor_data.dump();
            websocketpp::lib::error_code ec;
            tc->send(hdl, message, websocketpp::frame::opcode::text, ec);
            if (ec) {
                std::cerr << "Send error: " << ec.message() << std::endl;
            }
        }
        delay();
    }

    std::cout << "Exiting send_json_loop_secure thread" << std::endl;
    logManager.setLogLevel(LogManager::INFO);
    logManager.log(LogManager::INFO, "Exiting send json loop secure thread");
}

void Ai_handle() {
    MultiLayerPerceptron<double> mlp2;
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Setting up AI model");

    mlp2.import_from_json("EdgeFrontier/model/model.json");
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Starting Ai_handle thread");

    while (is_run){
        {
            vector<vector<double>> inputs = {{static_cast<double>(rand()%2), static_cast<double>(rand()%2)}};
            mlp2.predict(inputs, R_D);
        }
        delay();
    }

    mlp2.clearModel();

    std::cout << "Exiting Ai_handle thread" << std::endl;
    logManager.setLogLevel(LogManager::INFO);
    logManager.log(LogManager::INFO, "Exiting AI handle thread");
}

void handle_machine(){
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Starting handle_machine thread");
    nlohmann::json HID_only;
    nlohmann::json pre_info;

    std::string HID, MODE, SPEED;
    while (is_run) {
        HID_only = {
            {"HardwareID", HardwareID}
        };
        std::cout << HID_only.dump(4) << std::endl;
        std::string res = http.post_json(rest_main_server_cstr + "/hardware", HID_only.dump());
        std::cout << "Response: " << res << std::endl;
        if (res != "") {
            pre_info = nlohmann::json::parse(res);
            if (pre_info["HardwareID"] == HardwareID) {
                MODE = pre_info["Mode"].get<std::string>();
                SPEED = pre_info["Speed"].get<std::string>();
                std::transform(HardwareID.begin(), HardwareID.end(), HardwareID.begin(), ::toupper);
                std::transform(MODE.begin(), MODE.end(), MODE.begin(), ::toupper);
                std::transform(SPEED.begin(), SPEED.end(), SPEED.begin(), ::toupper);
                if (MODE == "PREDICTION") {
                    // * check if current mode is not prediction mode then switch to prediction mode
                    if (current_mode != PREDICTION_MODE) {
                        current_mode = PREDICTION_MODE;
                        logManager.setLogLevel(LogManager::INFO);
                        logManager.log(LogManager::INFO, "Switching to PREDICTION mode");
                    }
                } else {
                    // * check if current mode is not safe mode then switch to safe mode
                    if (current_mode != SAFE_MODE) {
                        current_mode = SAFE_MODE;
                        logManager.setLogLevel(LogManager::INFO);
                        logManager.log(LogManager::INFO, "Switching to SAFE mode");
                    }
                }
                if (SPEED == "SLOW") {
                    if (current_speed != SLOW) {
                        current_speed = SLOW;
                        logManager.setLogLevel(LogManager::INFO);
                        logManager.log(LogManager::INFO, "Setting speed to SLOW");
                    }
                } else if (SPEED == "MEDIUM") {
                    if (current_speed != MEDIUM) {
                        current_speed = MEDIUM;
                        logManager.setLogLevel(LogManager::INFO);
                        logManager.log(LogManager::INFO, "Setting speed to MEDIUM");
                    }
                } else {
                    if (current_speed != FAST) {
                        current_speed = FAST;
                        logManager.setLogLevel(LogManager::INFO);
                        logManager.log(LogManager::INFO, "Setting speed to FAST");
                    }
                }
            } else {
                std::cerr << "HardwareID not match" << std::endl;
                logManager.setLogLevel(LogManager::ERR);
                logManager.log(LogManager::ERR, "HardwareID not match");
            }
        }
        delay_server();
    }
    logManager.setLogLevel(LogManager::INFO);
    logManager.log(LogManager::INFO, "Exiting handle_machine thread");
}

/**
 * @brief Handles the on_open event for a non-secure WebSocket connection.
 * 
 * This function starts a separate thread for sending the sensor data JSON to the WebSocket server.
 * 
 * @param c A pointer to the WebSocket client.
 * @param hdl The WebSocket connection handle.
 **/
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
 **/
void on_open_secure(tls_client* tc, websocketpp::connection_hdl hdl) {
    std::thread send_thread(send_json_loop_secure, tc, hdl);
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
 **/
std::shared_ptr<boost::asio::ssl::context> on_tls_init(websocketpp::connection_hdl) {
    std::shared_ptr<boost::asio::ssl::context> ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv12_client));
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cerr << "TLS error: " << e.what() << std::endl;
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "TLS error: " + std::string(e.what()));
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
 **/
void handle_no_secure(const std::string &uri, client *c)
{
    try
    {
        // * clean uri before use
        std::string uri_clean = uri;
        if (uri_clean.find_first_of("\t\n\r\f\v") != std::string::npos)
        {
            uri_clean.erase(uri_clean.find_first_of("\t\n\r\f\v"));
            logManager.setLogLevel(LogManager::DEBUG);
            logManager.log(LogManager::DEBUG, "Deleted \\t\\n\\r\\f\\v from uri and now uri is clean");
        }
        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "Setting up non-secure WebSocket connection");
        // * Set logging settings for WebSocket client
        c->set_error_channels(websocketpp::log::elevel::none);
        c->set_access_channels(websocketpp::log::alevel::none);
        // * Initialize ASIO for WebSocket client
        c->init_asio();
        // * Set the open handler for the WebSocket connection
        c->set_open_handler(websocketpp::lib::bind(&on_open, c, websocketpp::lib::placeholders::_1));
        // * Create connection to WebSocket server
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c->get_connection(uri_clean, ec);
        if (ec)
        {
            std::cerr << "Error: " << ec.message() << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "Error: " + ec.message());
            return;
        }
        // * Connect to the server
        c->connect(con);
        logManager.setLogLevel(LogManager::INFO);
        logManager.log(LogManager::INFO, "Connected to WebSocket server");
        // * Start a separate thread for WebSocket client to handle the connection
        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "Starting WebSocket client thread");
        std::thread websocket_thread([c]()
                                     {
                                         c->run(); // * Run the WebSocket client
                                     });
        // * Start threads for updating and printing the sensor data
        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "Starting data is_run with threads");
        std::thread update_thread(update_json_loop);
        std::thread print_thread(print_json);
        std::thread Ai_thread(Ai_handle);
        // * Wait for threads to finish
        if (update_thread.joinable()) {
            update_thread.join();
            std::cout << "update_thread joined" << std::endl;
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "Updating sensor data thread joined");
        } else {
            std::cerr << "update_thread is not joinable" << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "Updating sensor data thread is not joinable");
        }

        if (print_thread.joinable()) {
            print_thread.join();
            std::cout << "print_thread joined" << std::endl;
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "Printing sensor data thread joined");
        } else {
            std::cerr << "print_thread is not joinable" << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "Printing sensor data thread is not joinable");
        }

        if (Ai_thread.joinable()) {
            Ai_thread.join();
            std::cout << "Ai_thread joined" << std::endl;
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "AI thread joined");
        } else {
            std::cerr << "Ai_thread is not joinable" << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "AI thread is not joinable");
        }

        if (!is_run) {
            c->close(con->get_handle(), websocketpp::close::status::normal, "User requested disconnect");
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "Disconnected from WebSocket server");
        }

        websocket_thread.join();
        std::cout << "websocket_thread joined" << std::endl;
        logManager.setLogLevel(LogManager::INFO);
        logManager.log(LogManager::INFO, "WebSocket client thread joined");
    }
    catch (const websocketpp::exception &e)
    {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "WebSocket error: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "Error: " + std::string(e.what()));
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
 *       - One for is_run the WebSocket client.
 *       - One for updating sensor data.
 *       - One for printing sensor data.
 *
 * @throws websocketpp::exception If a WebSocket error occurs.
 * @throws std::exception If a general error occurs.
 **/
void handle_secure(const std::string &uri, tls_client *tc)
{
    try
    {
        // * clean uri before use
        std::string uri_clean = uri;
        // uri_clean.erase(uri_clean.find_last_not_of("\t\n\r\f\v") + 1);
        // * check uri have "\t\n\r\f\v" or not If have remove it
        if (uri_clean.find_first_of("\t\n\r\f\v") != std::string::npos)
        {
            uri_clean.erase(uri_clean.find_first_of("\t\n\r\f\v"));
            logManager.setLogLevel(LogManager::DEBUG);
            logManager.log(LogManager::DEBUG, "Deleted \\t\\n\\r\\f\\v from uri and now uri is clean");
        }

        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "Setting up secure WebSocket connection");
        // * Set logging settings for WebSocket client
        tc->set_error_channels(websocketpp::log::elevel::none);
        tc->set_access_channels(websocketpp::log::alevel::none);
        // * Initialize ASIO for WebSocket client
        tc->init_asio();
        // * Set the TLS context initialization handler
        tc->set_tls_init_handler(websocketpp::lib::bind(&on_tls_init, websocketpp::lib::placeholders::_1));
        // * Set the open handler for the WebSocket connection
        tc->set_open_handler(websocketpp::lib::bind(&on_open_secure, tc, websocketpp::lib::placeholders::_1));
        
        // * Create connection to WebSocket server
        websocketpp::lib::error_code ec;
        std::cout << "Connecting to secure WebSocket server at " << uri_clean << std::endl;
        tls_client::connection_ptr con = tc->get_connection(uri_clean, ec);
        if (ec)
        {
            std::cerr << "Error: " << ec.message() << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "Error: " + ec.message());
            return;
        }
        // * Connect to the server
        tc->connect(con);
        std::thread websocket_thread([tc]()
                                        {
                                            tc->run(); // * Run the WebSocket client
                                        });
        logManager.setLogLevel(LogManager::INFO);
        logManager.log(LogManager::INFO, "Connected to secure WebSocket server");
        // * Start a separate thread for WebSocket client to handle the connection
        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "Starting WebSocket client thread");
        // * Start threads for updating and printing the sensor data
        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "Starting data is_run with threads");
        std::thread update_thread(update_json_loop);
        std::thread print_thread(print_json);
        std::thread Ai_thread(Ai_handle);
        // * Wait for threads to finish
        if (update_thread.joinable()) {
            update_thread.join();
            std::cout << "update_thread joined" << std::endl;
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "Updating sensor data thread joined");
        } else {
            std::cerr << "update_thread is not joinable" << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "Updating sensor data thread is not joinable");
        } 

        if (print_thread.joinable()) {
            print_thread.join();
            std::cout << "print_thread joined" << std::endl;
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "Printing sensor data thread joined");
        } else {
            std::cerr << "print_thread is not joinable" << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "Printing sensor data thread is not joinable");
        }

        if (Ai_thread.joinable()) {
            Ai_thread.join();
            std::cout << "Ai_thread joined" << std::endl;
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "AI thread joined");
        } else {
            std::cerr << "Ai_thread is not joinable" << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "AI thread is not joinable");
        }

        if (!is_run) {
            tc->close(con->get_handle(), websocketpp::close::status::normal, "User requested disconnect");
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "Disconnected from secure WebSocket server");
        }

        if (websocket_thread.joinable()) {
            websocket_thread.join();
            std::cout << "websocket_thread joined" << std::endl;
            logManager.setLogLevel(LogManager::INFO);
            logManager.log(LogManager::INFO, "WebSocket client thread joined");
        } else {
            std::cerr << "websocket_thread is not joinable" << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "WebSocket client thread is not joinable");
        }

        std::cout << "websocket_thread joined" << std::endl;
        logManager.setLogLevel(LogManager::INFO);
        logManager.log(LogManager::INFO, "WebSocket client thread joined");

    } catch (const websocketpp::exception &e) {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "WebSocket error: " + std::string(e.what()));
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "Error: " + std::string(e.what()));
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
 **/
std::unordered_map<std::string, std::string> loadEnvFile(const std::string& filePath) {
    std::unordered_map<std::string, std::string> envMap;
    std::ifstream envFile(filePath);
    if (!envFile.is_open()) {
        std::cerr << "Error: Unable to open .env file: " << filePath << std::endl;
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "Unable to open .env file: " + filePath);
        return envMap;
    }

    std::string line;
    while (std::getline(envFile, line)) {
        // Skip comments or empty lines
        if (line.empty() || line[0] == '#') {
            logManager.setLogLevel(LogManager::DEBUG);
            logManager.log(LogManager::DEBUG, "Skipping comment or empty line in .env file");
            continue;
        }

        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) {
            std::cerr << "Error: Malformed line in .env file: " << line << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "Malformed line in .env file: " + line);
            continue;
        }

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "Loaded .env KEY : " + key);

        // Trim whitespace (optional, implement trim logic if needed)
        envMap[key] = value;

        // Optionally set the variable in the environment
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
 */
bool is_secure(const std::string &uri)
{
    bool is_secure = uri.substr(0, 3) == "wss";
    if (is_secure)
    {
        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "WebSocket connection is secure");
    }
    else
    {
        logManager.setLogLevel(LogManager::WARNING);
        logManager.log(LogManager::WARNING, "WebSocket connection is not secure");
    }
    return is_secure;
}

int main(int argc, char *argv[]){
    std::cout << "EdgeFrontier - Sensor Data Simulator" << std::endl;
    std::cout << "Press 't' or 'T' to quit the program." << std::endl;
    std::cout << "Press 'm' or 'M' to change mode." << std::endl;

    for (int i = 0; i < argc; i++) {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
    }
    // * Set the log file for the log manager
    logManager.setLogFile("EdgeFrontier/log/activity.log");
    // * Load environment variables from a .env file
    std::string envFilePath = "EdgeFrontier/env/dev.env";
    std::unordered_map<std::string, std::string> envMap;

    std::string response = "";
    std::transform(HardwareID.begin(), HardwareID.end(), HardwareID.begin(), ::toupper);
    // TODO Check if the response is empty or not
    while (HardwareID == "UNKNOWN") {
        envMap = loadEnvFile(envFilePath);
        // * Check if the REST_MAIN_SERVER environment variable is set
        rest_main_server_cstr = getenv("REST_MAIN_SERVER");
        if (rest_main_server_cstr.empty()) {
            std::cerr << "Error: REST_MAIN_SERVER environment variable is not set." << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "REST_MAIN_SERVER environment variable is not set.");
            return 1;
        }
        // TODO I recommend to remove this "\t\n\r\f\v" at the end of the string
        rest_main_server_cstr.erase(rest_main_server_cstr.find_last_not_of("\t\n\r\f\v") + 1);

        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "REST_MAIN_SERVER environment variable is set.");

        std::cout << "REST_MAIN_SERVER: " << rest_main_server_cstr << std::endl;
        response = http.get(rest_main_server_cstr + "/register");
        if (response == "") {
            std::cerr << "Error: Unable to connect to the main server." << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "Unable to connect to the main server.");
            std::this_thread::sleep_for(std::chrono::microseconds(200000));
        }

        if (response != "") {
            nlohmann::json pre_info = nlohmann::json::parse(response);
            if (pre_info["HardwareID"] != "UNKNOWN") {
                HardwareID = pre_info["HardwareID"].get<std::string>();
                std::transform(HardwareID.begin(), HardwareID.end(), HardwareID.begin(), ::toupper);
                std::cout << "HardwareID: " << HardwareID << std::endl;
                logManager.setLogLevel(LogManager::INFO);
                logManager.log(LogManager::INFO, "HardwareID is set.");
            }
        } else {
            std::cerr << "Error: HardwareID is not set." << std::endl;
            logManager.setLogLevel(LogManager::ERR);
            logManager.log(LogManager::ERR, "HardwareID is not set.");
        }
    }

    // ! if rest api is http log warning
    if (rest_main_server_cstr.find("https") == std::string::npos) {
        logManager.setLogLevel(LogManager::WARNING);
        logManager.log(LogManager::WARNING, "REST API is not secure");
    } else {
        logManager.setLogLevel(LogManager::DEBUG);
        logManager.log(LogManager::DEBUG, "REST API is secure");
    }

    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Environment variables loaded from .env file.");

    // * Check if the WS_URI environment variable is set
    std::string ws_uri_cstr = envMap["WS_URI"];
    if (ws_uri_cstr.empty()) {
        std::cerr << "Error: WS_URI environment variable is not set." << std::endl;
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "WS_URI environment variable is not set.");
        return 1;
    }
    // TODO I recommend to remove this "\t\n\r\f\v" at the end of the string
    ws_uri_cstr.erase(ws_uri_cstr.find_last_not_of("\t\n\r\f\v") + 1);

    std::string uri(ws_uri_cstr);
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "WS_URI environment variable is set.");

    std::cout << "WS_URI: " << uri << std::endl;

    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Connecting to WebSocket server");

    std::cout << "Connecting to WebSocket server at: " << uri << std::endl;
    client c;
    tls_client tc;
    logManager.setLogLevel(LogManager::INFO);
    logManager.log(LogManager::INFO, "WebSocket client initialized.");
    
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Checking if WebSocket connection is secure.");

    std::thread machine_thread(handle_machine);
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Starting machine handle thread");

    // * is_run thread for checking input
    std::thread input_thread(checkInput_main);
    logManager.setLogLevel(LogManager::DEBUG);
    logManager.log(LogManager::DEBUG, "Starting input checking thread");

    // * Check if the WebSocket connection is secure
    is_secure(uri) ? handle_secure(ws_uri_cstr, &tc) : handle_no_secure(ws_uri_cstr, &c);

    std::cout << "Exiting main thread" << std::endl;
    logManager.setLogLevel(LogManager::INFO);
    logManager.log(LogManager::INFO, "Exiting main thread");

    // * Wait for the input thread to finish
    if (input_thread.joinable()) {
        input_thread.join();
        logManager.setLogLevel(LogManager::INFO);
        logManager.log(LogManager::INFO, "Input checking thread joined");
    } else {
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "Input checking thread is not joinable");
    }
    
    // * Wait for the machine handle thread to finish
    if (machine_thread.joinable()) {
        machine_thread.join();
        logManager.setLogLevel(LogManager::INFO);
        logManager.log(LogManager::INFO, "Machine handle thread joined");
    } else {
        logManager.setLogLevel(LogManager::ERR);
        logManager.log(LogManager::ERR, "Machine handle thread is not joinable");
    }
    
    return 0;
}
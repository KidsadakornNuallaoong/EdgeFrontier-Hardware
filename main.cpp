#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/common/thread.hpp>

typedef websocketpp::client<websocketpp::config::asio_client> client;

// Global mutex for thread safety
std::mutex mtx;

// Random number generator for sensor data simulation
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 100.0);

const char* Event[] = {"Cold", "Warm", "Hot", "Dry", "Wet", "Normal", "Unknown"};

// JSON structure holding initial sensor data
nlohmann::json sensor_data = {
    {"TimeStamp", "18/11/24 08:06:59"},
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

// Function to update sensor data with new random values
void update_sensor_data(nlohmann::json& j) {
    std::lock_guard<std::mutex> lock(mtx); // Ensuring thread safety
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%d/%m/%y %H:%M:%S");
    j["TimeStamp"] = ss.str();
    j["Event"] = Event[rand()%7];
    j["Data"]["CO2"] = dis(gen);
    j["Data"]["VOC"] = dis(gen);
    j["Data"]["RA"] = dis(gen);
    j["Data"]["TEMP"] = dis(gen);
    j["Data"]["HUMID"] = dis(gen);
    j["Data"]["PRESSURE"] = dis(gen);
}

// Function to print the current sensor data in JSON format every second
void print_json() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx); // Ensuring thread safety
            std::cout << sensor_data.dump(4) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Sleep for 1 second before printing again
    }
}

// Function to continuously update sensor data in a loop
void update_json_loop() {
    while (true) {
        update_sensor_data(sensor_data);
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Sleep for 1 second before updating again
    }
}

// Function to continuously send sensor data to the WebSocket server
void send_json_loop(client* c, websocketpp::connection_hdl hdl) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx); // Ensuring thread safety
            std::string message = sensor_data.dump(); // Serialize the JSON message
            c->send(hdl, message, websocketpp::frame::opcode::text); // Send the JSON message to the server
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Sleep for 1 second before sending again
    }
}

// WebSocket open handler function
void on_open(client* c, websocketpp::connection_hdl hdl) {
    // Start a thread to continuously send sensor data to the server
    std::thread send_thread(send_json_loop, c, hdl);
    send_thread.detach(); // Detach the thread to run independently
}

int main() {
    // WebSocket client initialization
    client c;

    try {
        // Set logging settings for WebSocket client
        c.set_error_channels(websocketpp::log::elevel::none);
        c.set_access_channels(websocketpp::log::alevel::none);

        // Initialize ASIO for WebSocket client
        c.init_asio();

        // Set the open handler for the WebSocket connection
        c.set_open_handler(websocketpp::lib::bind(&on_open, &c, websocketpp::lib::placeholders::_1));

        // Create connection to WebSocket server
        std::string uri = "ws://localhost:8181";
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);

        if (ec) {
            std::cerr << "Error: " << ec.message() << std::endl;
            return 1;
        }

        // Connect to the server
        c.connect(con);

        // Start a separate thread for WebSocket client to handle the connection
        std::thread websocket_thread([&c]() {
            c.run(); // Run the WebSocket client
        });

        // Start threads for updating and printing the sensor data
        std::thread update_thread(update_json_loop);
        std::thread print_thread(print_json);

        // Wait for threads to finish
        update_thread.join();
        print_thread.join();
        websocket_thread.join();

    } catch (const websocketpp::exception& e) {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
    }

    return 0;
}
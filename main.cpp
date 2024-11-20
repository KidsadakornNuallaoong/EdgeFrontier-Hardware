#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio.hpp>
#include <boost/asio/ssl/context.hpp>

typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;

// Global mutex for thread safety
std::mutex mtx;

// Random number generator for sensor data simulation
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 100.0);

const char* Event[] = {"Cold", "Warm", "Hot", "Dry", "Wet", "Normal", "Unknown"};
const char* HardwareID[] = {"EF-001", "EF-002"};

// JSON structure holding initial sensor data
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

// Function to update sensor data with new random values
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

void print_json() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << sensor_data.dump(4) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void update_json_loop() {
    while (true) {
        update_sensor_data(sensor_data);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void send_json_loop(tls_client* c, websocketpp::connection_hdl hdl) {
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
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void on_open(tls_client* c, websocketpp::connection_hdl hdl) {
    std::thread send_thread(send_json_loop, c, hdl);
    send_thread.detach();
}

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

int main() {
    tls_client c;

    try {
        c.set_error_channels(websocketpp::log::elevel::none);
        c.set_access_channels(websocketpp::log::alevel::none);
        c.init_asio();

        c.set_tls_init_handler(websocketpp::lib::bind(&on_tls_init, websocketpp::lib::placeholders::_1));
        c.set_open_handler(websocketpp::lib::bind(&on_open, &c, websocketpp::lib::placeholders::_1));

        std::string uri = "wss://localhost:9002";
        websocketpp::lib::error_code ec;
        tls_client::connection_ptr con = c.get_connection(uri, ec);

        if (ec) {
            std::cerr << "Connection error: " << ec.message() << std::endl;
            return 1;
        }

        c.connect(con);

        std::thread websocket_thread([&c]() { c.run(); });
        std::thread update_thread(update_json_loop);
        std::thread print_thread(print_json);

        update_thread.join();
        print_thread.join();
        websocket_thread.join();

    } catch (const websocketpp::exception& e) {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
    }

    return 0;
}

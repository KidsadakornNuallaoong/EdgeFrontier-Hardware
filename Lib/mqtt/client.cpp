#include <iostream>
#include "mqtt.h"

const char *host = "localhost";  // Change to broker IP if needed
int port = 1883;
const char *topic = "test";
const char *message = "Hello from Mosquitto MQTT Publisher!";

int main() {
    // Example usage
    std::thread pub_thread(mqtt_publish, host, port, topic, message);
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Allow some time for the publish to occur

    pub_thread.join(); // Wait for the publisher thread to finish
    return 0;
}
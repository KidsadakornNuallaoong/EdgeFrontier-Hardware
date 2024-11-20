#include <iostream>
#include "mqtt.h"

const char *host = "localhost";  // Change to broker IP if needed
int port = 1883;
const char *topic = "test";
const char *message = "Hello from Mosquitto MQTT Publisher!";

int main() {
    mqtt_subscribe(host, port, topic);
    return 0;
}
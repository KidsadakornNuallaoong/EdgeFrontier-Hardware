#if !defined(MQTT_H)
#define MQTT_H

#include <iostream>
#include <mosquitto.h>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>

std::string received_message;

#ifdef _WIN32
#include <winsock2.h>  // Windows socket library
#pragma comment(lib, "ws2_32.lib") // Link with Winsock library on Windows
#endif

void message_callback(mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    if (msg->payloadlen) {
        std::string topic = msg->topic;
        std::string payload = std::string((char *)msg->payload, msg->payloadlen);
        std::cout << "Received message on topic " << topic << ": " << payload << std::endl;
        received_message = payload;  // Store the received message
    }
}

void mqtt_publish(const char* host, int port, const char* topic, const char* message) {
    // Initialize Winsock on Windows
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto client instance
    mosquitto *mosq = mosquitto_new(nullptr, true, nullptr);
    if (!mosq) {
        std::cerr << "Failed to create Mosquitto client instance." << std::endl;
        mosquitto_lib_cleanup(); // Clean up library
        return;
    }

    // Connect to the MQTT broker
    if (mosquitto_connect(mosq, host, port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to broker." << std::endl;
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup(); // Clean up library
        return;
    }

    // Publish a message to the MQTT topic
    int ret = mosquitto_publish(mosq, nullptr, topic, strlen(message), message, 0, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish message: " << mosquitto_strerror(ret) << std::endl;
    } else {
        std::cout << "Message published to topic " << topic << std::endl;
    }

    // Disconnect from the broker
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup(); // Clean up library

#ifdef _WIN32
    // Cleanup Winsock on Windows
    WSACleanup();
#endif
}

void mqtt_subscribe(const char* host, int port, const char* topic) {
    // Initialize Winsock on Windows
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto client instance
    mosquitto *mosq = mosquitto_new(nullptr, true, nullptr);
    if (!mosq) {
        std::cerr << "Failed to create Mosquitto client instance." << std::endl;
        mosquitto_lib_cleanup(); // Clean up library
        return;
    }

    // Set the message callback function
    mosquitto_message_callback_set(mosq, message_callback);

    // Connect to the MQTT broker
    if (mosquitto_connect(mosq, host, port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to broker." << std::endl;
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup(); // Clean up library
        return;
    }

    // Subscribe to the MQTT topic
    int ret = mosquitto_subscribe(mosq, nullptr, topic, 0);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to subscribe to topic: " << mosquitto_strerror(ret) << std::endl;
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup(); // Clean up library
        return;
    }

    // Loop to handle incoming messages
    mosquitto_loop_start(mosq); // Start a thread to process network events

    // Wait for a message to be received
    while (received_message.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for a short time
    }

    // Cleanup
    mosquitto_loop_stop(mosq, true);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup(); // Clean up library

#ifdef _WIN32
    // Cleanup Winsock on Windows
    WSACleanup();
#endif
}

#endif // MQTT_H

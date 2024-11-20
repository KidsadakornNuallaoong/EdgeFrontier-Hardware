#include <iostream>
#include <thread>
#include <sio/sio_client.h>
#include <sio/sio_message.h>
#include <sio/sio_socket.h>

using namespace std;

// * Create a global client instance
sio::client h;

// * Function to handle incoming messages
void on_message(sio::event& ev) {
    cout << "Received message: " << ev.get_message()->get_string() << endl;
}

int main() {
    // * Set up listeners for the socket connection events
    h.set_open_listener([]() {
        cout << "Connected to server" << endl;
    });

    h.set_close_listener([](sio::client::close_reason const& reason) {
        cout << "Disconnected from server" << endl;
    });

    h.set_fail_listener([]() {
        cout << "Connection failed" << endl;
    });

    // * Register the message event handler
    h.socket()->on("message", &on_message);

    // * Update the connection URL to the correct port
    h.connect("http://localhost:4000");  // * Change to port 4000

    // * Keep the program running to listen for events
    while (true) {
        sio::message::ptr msg = sio::string_message::create("Hello from client!");
        h.socket()->emit("message", msg);

        // * Sleep for a short time
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // * Close the connection gracefully
    h.close();

    return 0;
}

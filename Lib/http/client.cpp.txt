#include <iostream>
#include <curl/curl.h>

int main() {
    CURL *curl;
    CURLcode res;

    // Initialize curl
    curl = curl_easy_init();
    if(curl) {
        // Set the URL of the server
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3000");

        // Specify the data to be sent
        const char *postData = "{\"username\":\"JEZHKNKKJI4TCR32JF4U2U2FNFDDENLJOEYUS3KNJNLUETCKGF4UMVKJMJBW2OBPKB2D2PIK\", \"password\": \"IZKESNKKJI4TCRCLK54UO6RZGBCWCV3DJVFDK6DPKV4UM3SKOFRHCUJYLAFA====\"}"; // Example JSON data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);

        // Set the content type to application/json
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            std::cout << "Data sent successfully!" << std::endl; // Success message
        }

        // Cleanup
        curl_slist_free_all(headers); // Free the headers list
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize curl." << std::endl;
    }

    return 0;
}

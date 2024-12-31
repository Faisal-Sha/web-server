#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>  // Include pthread library for multi-threading

#define PORT 8080
#define WWW_DIR "www"  // Directory where the HTML files are stored

// Function to handle client requests
void* handleClient(void* clientSocketPtr) {
    int clientSocket = *((int*)clientSocketPtr);
    delete (int*)clientSocketPtr;  // Clean up the allocated memory for client socket

    char buffer[1024];

    // Read the request from the client
    int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        std::cerr << "Error reading from client socket" << std::endl;
        return nullptr;
    }

    buffer[bytesRead] = '\0';  // Null terminate the buffer to make it a string

    // Print the request for debugging
    std::cout << "Received request:\n" << buffer << std::endl;

    // Extract the requested path (we assume a simple GET request for now)
    std::string request(buffer);
    size_t pos = request.find(' ');  // Find the first space after "GET"
    size_t endPos = request.find(' ', pos + 1);  // Find the second space (end of the URL)

    std::string requestedPath = request.substr(pos + 1, endPos - pos - 1);
    if (requestedPath == "/") {
        requestedPath = "/index.html";  // Default to index.html for "/"
    }

    // Construct the full path to the file
    std::string filePath = WWW_DIR + requestedPath;

    // Open the requested file
    std::ifstream file(filePath);
    std::string response;

    if (file.is_open()) {
        // File found, read the content and send it as response
        std::stringstream buffer;
        buffer << file.rdbuf();  // Read the entire file into a string

        // Prepare the HTTP response with the file content
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + buffer.str();
    } else {
        // File not found, return 404 response
        response = "HTTP/1.1 404 Not Found\r\n\r\n"
                   "<html><body><h1>404 Not Found</h1></body></html>";
    }

    // Send the HTTP response
    send(clientSocket, response.c_str(), response.length(), 0);

    // Close the client socket after responding
    close(clientSocket);

    return nullptr;
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create the socket (IPv4, TCP)
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Set up the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    // Bind the socket to the IP and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 7) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    // Accept incoming connections and handle them with threads
    while (true) {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        // Create a new thread to handle the client
        pthread_t threadId;
        int* clientSocketPtr = new int(clientSocket);  // Dynamically allocate memory for clientSocket
        if (pthread_create(&threadId, nullptr, handleClient, (void*)clientSocketPtr) != 0) {
            std::cerr << "Error creating thread" << std::endl;
            delete clientSocketPtr;  // Clean up memory if thread creation fails
        } else {
            // Detach the thread so it can clean up itself after it finishes
            pthread_detach(threadId);
        }
    }

    // Close the server socket (not reached due to the infinite loop)
    close(serverSocket);
    return 0;
}

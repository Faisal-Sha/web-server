#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

// Function to handle client requests
void handleClient(int clientSocket) {
    char buffer[1024];

    // Read the request from the client
    int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        std::cerr << "Error reading from client socket" << std::endl;
        return;
    }

    buffer[bytesRead] = '\0';  // Null terminate the buffer to make it a string

    // Print the request for debugging
    std::cout << "Received request:\n" << buffer << std::endl;

    // Extract the requested path (we assume a simple GET request for now)
    std::string request(buffer);
    size_t pos = request.find(' ');  // Find the first space after "GET"
    size_t endPos = request.find(' ', pos + 1);  // Find the second space (end of the URL)

    std::string requestedPath = request.substr(pos + 1, endPos - pos - 1);

    // Prepare the HTTP response
    std::string response = "HTTP/1.1 200 OK\r\n\r\nRequested path: " + requestedPath + "\r\n";

    // Send the HTTP response
    send(clientSocket, response.c_str(), response.length(), 0);

    // Close the client socket after responding
    close(clientSocket);
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
    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    // Accept incoming connections
    while (true) {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        // Handle the client request
        handleClient(clientSocket);
    }

    // Close the server socket (not reached due to the infinite loop)
    close(serverSocket);
    return 0;
}

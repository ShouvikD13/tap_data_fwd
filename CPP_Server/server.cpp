#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstring>

#define PORT 8080

void printHex(const char* buffer, int length) {
    std::cout << "Hex Dump: ";
    for (int i = 0; i < length; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<unsigned int>(static_cast<unsigned char>(buffer[i]))) << " ";
    }
    std::cout << std::dec << std::endl; // Reset to decimal format
}

void startServer() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    char buffer[1024] = {0};

    // Create a socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Define the address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) == -1) {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        // Accept an incoming connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen)) == -1) {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            continue; // Go back to listening for new connections
        }

        std::cout << "Connection accepted" << std::endl;

        while (true) {
            // Read data from the client
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead == -1) {
                std::cerr << "Receive failed: " << strerror(errno) << std::endl;
                break;
            } else if (bytesRead == 0) {
                std::cout << "Client disconnected" << std::endl;
                break; // Client has disconnected, exit inner loop
            }

            std::cout << "Received " << bytesRead << " bytes" << std::endl;

            // Print the raw byte data
            printHex(buffer, bytesRead);

            // Send the binary data back to the client exactly as received
            int bytesSent = send(clientSocket, buffer, bytesRead, 0);
            if (bytesSent == -1) {
                std::cerr << "Send failed: " << strerror(errno) << std::endl;
                break;
            }

            std::cout << "Sent " << bytesSent << " bytes back to client" << std::endl;
        }

        // Close the connection
        close(clientSocket);
    }

    // Cleanup
    close(serverSocket);
}

int main() {
    startServer();
    return 0;
}

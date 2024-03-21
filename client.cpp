#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>

#define PORT 8080
#define BUFFER_SIZE 1024
#define END_EDIT "END_EDIT\n" // Znacznik końca edycji

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        error("Invalid address/ Address not supported");
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("Connection Failed");
    }

    while (true) {
        valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            error("Read error");
        }
        
        std::cout << "Server response:" << std::endl;
        std::cout << buffer << std::endl;
        
        std::cout << "Select action (create(1)/edit(2)/delete(3)): ";
        std::string action;
        std::cin >> action;

        send(sock, action.c_str(), action.length(), 0);

        
        switch (std::stoi(action)) {
            case 1: { 
                std::cout << "Enter filename: ";
                std::string filename;
                std::cin >> filename;
                send(sock, filename.c_str(), filename.length(), 0);
                std::cout << "File created: " << filename << std::endl;
                continue;
            }
            case 2: { 
                std::cout << "Enter filename: ";
                std::string filename;
                std::cin >> filename;
                send(sock, filename.c_str(), filename.length(), 0);
                std::cout << "Enter file content (type empty line to finish):\n";
                std::string content;
                std::cin.ignore(); 
                while (true) {
                    std::string line;
                    std::getline(std::cin, line);
                    if (line.empty()) { 
                        break;
                    }
                    content += line + "\n";
                }
                send(sock, content.c_str(), content.size(), 0);
		std::cout << "File edited: " << filename << std::endl;
                continue;
            }
            case 3: { 
                std::cout << "Enter filename: ";
                std::string filename;
                std::cin >> filename;
                send(sock, filename.c_str(), filename.length(), 0);
                std::cout << "File deleted: " << filename << std::endl;
                continue;
            }
            default:
                std::cerr << "Invalid action" << std::endl;
        	continue;
        }
        
        valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            error("Read error");
        }
        
        std::cout << "Server response: " << buffer << std::endl;
        
    }

    return 0;
}

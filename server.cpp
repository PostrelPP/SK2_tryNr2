#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

#define PORT 8080
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

std::vector<std::string> get_txt_files() {
    std::vector<std::string> files;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string filename = ent->d_name;
            if (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".txt") {
                files.push_back(filename);
            }
        }
        closedir(dir);
    } else {
        error("Failed to open directory");
    }
    return files;
}

void send_file_list(int client_socket) {
    std::vector<std::string> files = get_txt_files();
    std::string file_list;
    for (const std::string& filename : files) {
        file_list += filename + "\n";
    }
    std::cout << "Available files:\n" << file_list << std::endl;
    send(client_socket, file_list.c_str(), file_list.length(), 0);
}


void create_file(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        error("Failed to create file");
    }
    file.close();
    std::cout << "File created: " << filename << std::endl;
}

void modify_file(const std::string& filename, int client_socket) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        error("Failed to open file");
    }

    char buffer[BUFFER_SIZE];
    int valread;
    while ((valread = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[valread] = '\0'; 
        file << buffer;
    }
    if (valread == 0) {
        std::cout << "Client disconnected during file modification" << std::endl;
    } else {
        error("Failed to receive data from client");
    }
    file.close();
    std::cout << "File modified: " << filename << std::endl;
}

void delete_file(const std::string& filename) {
    if (remove(filename.c_str()) != 0) {
        error("Failed to delete file");
    }
    std::cout << "File deleted: " << filename << std::endl;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};

    send_file_list(client_socket);

    while (true) {
        int valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            if (valread == 0) {
                std::cout << "Client disconnected" << std::endl;
            } else {
                error("read error");
            }
            close(client_socket);
            return;
        }

        std::string action(buffer);
        
        valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            error("read error");
        }
        std::string filename(buffer);

        switch (std::stoi(action)) {
            case 1:
                create_file(filename);
                send_file_list(client_socket);
                continue;
            case 2:
                modify_file(filename, client_socket);
                continue; 
            case 3:
                delete_file(filename);
                send_file_list(client_socket);
                continue;
            default:
                std::cerr << "Invalid action" << std::endl;
                continue;
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    std::vector<int> client_sockets;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        error("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        error("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) == -1) {
        error("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == -1) {
            error("accept");
            exit(EXIT_FAILURE);
        }
        
        std::cout << "Client connected" << std::endl;

        client_sockets.push_back(new_socket);
        handle_client(new_socket);
    }

    return 0;
}

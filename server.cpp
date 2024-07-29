#include "server.h"
#include <stdlib.h>
#include <iostream>
#include <cstring> 

TCPServer::TCPServer(char* ip_addr, int port)
{
    server_socket = socket(AF_INET, SOCK_STREAM, 0); 

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(port); 
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)); 
}

void TCPServer::accept_connections(int connections)
{
    listen(server_socket, connections);
    
    std::cout << "Waiting for connections..." << std::endl;

    int client_socket = accept(server_socket, nullptr, nullptr);
    if (client_socket < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
    }

    int BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE] = {0};

    for (;;) {
        recv(client_socket, buffer, sizeof(buffer), 0);
        std::cout << buffer << std::endl;
        respond(buffer, client_socket);   
    }
}

void TCPServer::respond(char* request, int client_socket)
{
    char* msg = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 24\n\nno servers are available";
    send(client_socket, msg, strlen(msg), 0);
    close(client_socket);
}
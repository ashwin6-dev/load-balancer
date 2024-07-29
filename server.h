#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class TCPServer {
    int server_socket;
    sockaddr_in server_addr;

public:
    TCPServer(char* ip_addr, int port);
    void accept_connections(int connections);
    void respond(char* request, int client_socket);
};
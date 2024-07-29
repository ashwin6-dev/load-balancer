#pragma once

#include "server.h"
#include <vector>
#include <unordered_map>

struct ServerAddr {
    int port;
    bool alive;
    char* ip;
};

class LoadBalancer : protected TCPServer {
    std::vector<ServerAddr*> server_addrs;
    std::unordered_map<int, ServerAddr*> ring;

public:
    LoadBalancer(char* ip_addr, int port, std::vector<ServerAddr*> addrs);
    int connect_to_server(ServerAddr* addr, int* status);
    void health_check();
    void accept_connections(int connections);
    ServerAddr* closest_server(int hash);
    void respond(char* buffer, int client_socket);
};
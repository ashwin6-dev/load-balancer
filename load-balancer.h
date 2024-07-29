#pragma once

#include "server.h"

class LoadBalancer : TCPServer {
public:
    LoadBalancer(char* ip_addr, int port, char** server_addrs) : TCPServer(ip_addr, port) {};
};
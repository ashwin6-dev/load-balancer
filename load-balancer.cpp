#include "load-balancer.h"
#include <functional>
#include <iostream>
#include <cstring>
#include <climits>
#include <thread>
#include <chrono>

LoadBalancer::LoadBalancer(char* ip_addr, int port, std::vector<ServerAddr*> addrs) : TCPServer(ip_addr, port), server_addrs(addrs)
{
    std::hash<std::string> hash_function;

    for (int i = 0; i < addrs.size(); i++) {
        ServerAddr* addr = addrs[i];
        std::string key = std::string(addr->ip) + ":" + std::to_string(addr->port);
        int hash = hash_function(key);
        ring[hash] = addr;
    }
}

int LoadBalancer::connect_to_server(ServerAddr* addr, int* status)
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_address; 
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(addr->port); 
    server_address.sin_addr.s_addr = inet_addr(addr->ip);

    *status = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    return client_socket;
}

void LoadBalancer::health_check()
{
    for (;;) {
        for (ServerAddr* addr : server_addrs) {
            int status;
            int client_socket = connect_to_server(addr, &status);

            if (status < 0) {
                addr->alive = false;
            }else {
                addr->alive = true;
            }

            close(client_socket);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void LoadBalancer::accept_connections(int connections)
{
    std::thread health_thread([this]() { this->health_check(); });
    health_thread.detach();

    listen(server_socket, connections);
    
    std::cout << "LB is waiting for connections..." << std::endl;

    for (;;) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
        }

        int BUFFER_SIZE = 4096;
        char buffer[BUFFER_SIZE] = {0};

        recv(client_socket, buffer, sizeof(buffer), 0);
        respond(buffer, client_socket);
    }
}

ServerAddr* LoadBalancer::closest_server(int hash) {
    int closest_hash = -1;
    int min_diff = INT_MAX;
    
    for (auto& pair : ring) {
        int pair_hash = pair.first;
        int diff = pair_hash - hash;

        if (diff < min_diff && diff >= 0 && ring[pair_hash]->alive) {
            closest_hash = pair_hash;
            min_diff = diff;
        }
    }

    if (closest_hash == -1) {
        for (auto& pair : ring) {
            int pair_hash = pair.first;

            if (ring[pair_hash]->alive) {
                closest_hash = pair_hash;
                break;
            }
        }
    }

    if (closest_hash == -1) {
        return nullptr;
    }

    ServerAddr* addr = ring[closest_hash];
    return addr;
}

void LoadBalancer::respond(char* request, int client_socket)
{
    std::hash<char*> hash_function;
    int hash = hash_function(request);

    ServerAddr* addr = closest_server(hash);

    if (addr == nullptr) {
        char* msg = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 24\n\nno servers are available";
        send(client_socket, msg, strlen(msg), 0);
        close(client_socket);
        return;
    }

    int status;
    int server_socket = connect_to_server(addr, &status);

    if (status < 0) {
        std::cerr << "unable to connect" << std::endl;
        addr->alive = false;
        respond(request, client_socket);
        return;
    }
    
    send(server_socket, request, strlen(request), 0);

    char server_response[10000];
    read(server_socket, server_response, sizeof(server_response));

    std::cout << server_response << std::endl;
    send(client_socket, server_response, strlen(server_response), 0);
    
    close(client_socket);
    close(server_socket);
}
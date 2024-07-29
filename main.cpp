#include "server.h"

int main() {
    TCPServer server("127.0.0.1", 8000);
    server.accept_connections(10);
}
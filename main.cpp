#include "load-balancer.h"
#include <stdlib.h>

int main() {
    ServerAddr* s1 = (ServerAddr*) malloc(sizeof(ServerAddr));
    s1->alive = true;
    s1->ip = "127.0.0.1";
    s1->port = 8080;

    ServerAddr* s2 = (ServerAddr*) malloc(sizeof(ServerAddr));
    s2->alive = true;
    s2->ip = "127.0.0.1";
    s2->port = 8001;

    LoadBalancer lb("127.0.0.1", 8000, std::vector<ServerAddr*> { s1, s2 });
    lb.accept_connections(10);
}
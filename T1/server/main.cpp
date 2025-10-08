#include <iostream>
#include "Server.h"
#include "utils.h"
#include "discovery.h"

int main(int argc, char* argv[]) {
    int port = getValidatedPort(argc, argv);
    if (port == -1) return 1;

    Server server(port);
    server.init(port);

    return 0;
}

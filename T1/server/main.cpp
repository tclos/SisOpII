#include <iostream>
#include "Server.h"
#include "utils.h"
#include "discovery.h"

int main(int argc, char* argv[]) {
    int port = getValidatedPort(argc, argv);
    if (port == -1) return 1;

    Server server(port);
    server.init(port);

    try {
        run_discovery_service_server(port, server);
    } catch (const std::runtime_error& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

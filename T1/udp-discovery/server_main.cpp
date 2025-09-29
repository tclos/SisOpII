#include "discovery.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <porta>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);

    try {
        run_discovery_service_server(port);
    } catch (const std::runtime_error& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
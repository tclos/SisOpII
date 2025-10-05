#include <iostream>
#include "Client.h"
#include "utils.h"
#include "discovery.h"
#include <string>

int main(int argc, char* argv[]) {
    int port = getValidatedPort(argc, argv);
    if (port == -1) return 1;

    try {
        std::string server_address = run_discovery_service_client(port);
        Client client(server_address);
        client.init();
    } catch (const std::runtime_error& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

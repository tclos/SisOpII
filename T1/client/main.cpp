#include <iostream>
#include "Client.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    int port = getValidatedPort(argc, argv);
    if (port == -1) return 1;

    try {
        Client client(port);
        client.init();

    } catch (const std::runtime_error& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

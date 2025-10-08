#include <iostream>
#include <signal.h>
#include "Client.h"
#include "utils.h"

Client* global_client = nullptr;

void signalHandler(int signum) {
    if (global_client != nullptr) {
        global_client->shutdown();
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);

    int port = getValidatedPort(argc, argv);
    if (port == -1) return 1;

    try {
        Client client(port);

        global_client = &client;

        client.init();

    } catch (const std::runtime_error& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Cliente encerrado." << std::endl;
    return 0;
}

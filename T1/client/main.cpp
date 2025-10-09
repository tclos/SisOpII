#include <iostream>
#include <signal.h>
#include "Client.h"
#include "clientInterface.h"
#include "utils.h"

ClientInterface* global_interface = nullptr;

void signalHandler(int signum) {
    if (global_interface != nullptr) {
        global_interface->shutdown();
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);

    int port = getValidatedPort(argc, argv);
    if (port == -1) return 1;

    Client client(port);
    ClientInterface interface(client);
    global_interface = &interface;

    interface.start();

    std::cout << "Cliente encerrado." << std::endl;
    return 0;
}
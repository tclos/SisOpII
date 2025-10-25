#include <iostream>
#include <signal.h>
#include "Client.h"
#include "clientInterface.h"
#include "utils.h"

//Valida a Porta
//cria objetos Client e ClientInterface - para que a interface possa chamar as funções (como descobrir o servidor ou enviar transação)
//chama interface.start() que inicia o programa pro usuário
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

#include <iostream>
#include "Server.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso correto: " << argv[0] << " <PORTA_UDP>" << std::endl;
        return 1;
    }

    int port;
    try {
        port = std::stoi(argv[1]);
        if (port <= 1024 || port > 65535) {
            std::cerr << "Erro: A porta deve estar em um intervalo válido (acima de 1024)." << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Erro: O parâmetro da porta não é um número válido." << std::endl;
        return 1;
    }

    Server server(port);
    server.init(port);

    return 0;
}

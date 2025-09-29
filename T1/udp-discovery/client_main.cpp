#include "discovery.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <porta>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);

    try {
        std::string server_address = run_discovery_service_client(port);

        // Exibindo a saída no formato especificado pelo trabalho
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        std::cout << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " server_addr " << server_address << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
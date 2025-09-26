#include "utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>

std::string getCurrentFormattedTime() {
    auto now = std::time(nullptr);
    std::tm tm_struct = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm_struct, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool validatePortArg(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso correto: " << argv[0] << " <PORTA_UDP>" << std::endl;
        return false;
    }
    return true;
}

bool validatePort(int port) {
    if (port <= 1024 || port > 65535) {
        std::cerr << "Erro: A porta deve estar em um intervalo válido (acima de 1024)." << std::endl;
        return false;
    }
    return true;
}

int getValidatedPort(int argc, char* argv[]) {
    if (!validatePortArg(argc, argv)) {
        return -1;
    }
    int port;
    try {
        port = std::stoi(argv[1]);
        if (!validatePort(port)) {
            return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Erro: O parâmetro da porta não é um número válido." << std::endl;
        return -1;
    }
    return port;
}
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include "Client.h"
#include "clientInterface.h"
#include "utils.h"

ClientInterface* global_interface = nullptr;

void signalHandler(int signum) {
    (void)signum;
    std::cout << "\n" << getCurrentFormattedTime() 
              << " [CLIENTE] Sinal de interrupção recebido (SIGINT/SIGTERM). Encerrando..." 
              << std::endl;
              
    if (global_interface != nullptr) {
        global_interface->shutdown();
    }
    
    // Dar tempo para threads terminarem
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    exit(0);
}

void setupSignalHandlers() {
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    // Registrar para SIGINT (Ctrl+C)
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Erro ao configurar handler para SIGINT");
    }
    
    // Registrar para SIGTERM
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Erro ao configurar handler para SIGTERM");
    }
    
    // Ignorar SIGPIPE (para evitar término por socket fechado)
    signal(SIGPIPE, SIG_IGN);
}

int main(int argc, char* argv[]) {
    std::cout << getCurrentFormattedTime() 
              << " [CLIENTE] Iniciando cliente..." << std::endl;
    
    setupSignalHandlers();

    int port = getValidatedPort(argc, argv);
    if (port == -1) {
        std::cerr << getCurrentFormattedTime() 
                  << " [CLIENTE] ERRO: Porta inválida. Encerrando." << std::endl;
        return 1;
    }

    try {
        Client client(port);
        ClientInterface interface(client);
        global_interface = &interface;

        std::cout << getCurrentFormattedTime() 
                  << " [CLIENTE] Interface iniciando..." << std::endl;
        
        interface.start();

        std::cout << getCurrentFormattedTime() 
                  << " [CLIENTE] Cliente encerrado normalmente." << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << getCurrentFormattedTime() 
                  << " [CLIENTE] ERRO CRÍTICO: " << e.what() << std::endl;
        return 1;
    }
}
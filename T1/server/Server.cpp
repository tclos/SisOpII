
#include "Server.h"
#include "serverInterface.h"
#include "ServerUDP.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

#define INITIAL_BALANCE 1000.0f

Server::Server(int port)
    : udp_port(port), num_transactions(0), total_transferred(0), total_balance(0) {}

int Server::getPort() const { return udp_port; }
int Server::getNumTransactions() const { return num_transactions; }
float Server::getTotalTransferred() const { return total_transferred; }
float Server::getTotalBalance() const { return total_balance; }

void Server::init(int port) {
    ServerUDP serverUdp(port);
    logInitialMessage(num_transactions, static_cast<int>(total_transferred), static_cast<int>(total_balance));
}

void Server::printClients() const {
    std::lock_guard<std::mutex> lock(clients_mutex);

    std::cout << "\n--- Tabela de Clientes Atuais ---" << std::endl;
    std::cout << std::left << std::setw(20) << "Endereço IP"
              << std::setw(25) << "Último ID recebido"
              << std::setw(15) << "Saldo Atual" << std::endl;
    std::cout << "-----------------------------------------------------" << std::endl;

    for (const auto& client : clients) {
        std::cout << std::left << std::setw(20) << client.getAddress()
                  << std::setw(25) << client.getLastRequest()
                  << std::fixed << std::setprecision(2) << client.getBalance() << std::endl;
    }
    std::cout << "-----------------------------------------------------\n" << std::endl;
}

void Server::addClient(const std::string& client_ip, float balance) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    std::vector<Client>::iterator it = std::find_if(clients.begin(), clients.end(), [&](const Client& c) {
        return c.getAddress() == client_ip;
    });

if (it == clients.end()) {
        Client newClient(client_ip, 0, balance);
        clients.push_back(newClient);
        
        this->total_balance += balance;
    } else {
        std::cout << "Cliente " << client_ip << " já está registado." << std::endl;
    }

    printClients();
}

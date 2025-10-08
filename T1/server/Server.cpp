
#include "Server.h"
#include "serverInterface.h"
#include "ServerUDP.h"
#include "discovery.h"
#include "transactions.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <arpa/inet.h>

#define INITIAL_BALANCE 1000.0f

Server::Server(int port)
    : udp_port(port), num_transactions(0), total_transferred(0), total_balance(0) {}

void Server::init(int port) {
    struct sockaddr_in client_addr;
    Packet received_packet;

    ServerUDP serverUDP(port);
    logInitialMessage(num_transactions, static_cast<int>(total_transferred), static_cast<int>(total_balance));

    if (serverUDP.createSocket() == false) {
        throw std::runtime_error("Erro ao criar o socket do servidor.");
    }

    serverUDP.configureBroadcast();
    serverUDP.setUpServerAddress();
    serverUDP.bindSocket();

    while (true) {
        int n = serverUDP.receivePacket(received_packet, client_addr);
        if (n <= 0) continue;

        std::cout << "packet type: " << ntohs(received_packet.type) << std::endl;

        switch (ntohs(received_packet.type)) {
            case TRANSACTION_REQ: {
                Packet req_copy = received_packet;
                handleTransactionRequest(req_copy, client_addr, *this, serverUDP);
                break;
            }
            case DISCOVERY: {
                handleDiscoveryPacket(serverUDP, *this, client_addr);
                break;
            }
            default:
                std::cerr << "Tipo de pacote desconhecido recebido." << std::endl;
                break;
        }
    }

    printClients();

    serverUDP.closeSocket();
}

void Server::printClients_unlocked() const {
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


void Server::printClients() const {
    std::lock_guard<std::mutex> lock(clients_mutex);
    printClients_unlocked();
}

void Server::addClient_unlocked(const std::string& client_ip) {
    std::vector<ClientDTO>::iterator it = std::find_if(clients.begin(), clients.end(), [&](const ClientDTO& c) {
        return c.getAddress() == client_ip;
    });

    if (it == clients.end()) {
            ClientDTO newClient(client_ip);
            clients.push_back(newClient);
            
            this->total_balance += newClient.getBalance();
    } else {
        std::cout << "Cliente " << client_ip << " já está registado." << std::endl;
    }
    
    printClients_unlocked();
}

std::vector<ClientDTO>::iterator Server::findClient(const std::string& ip) {
    return std::find_if(clients.begin(), clients.end(), [&](const ClientDTO& c) {
        return c.getAddress() == ip;
    });
}

void Server::addClient(const std::string& client_ip) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    addClient_unlocked(client_ip);
}

TransactionStatus Server::validateTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn) {
    if (source_it == clients.end() || dest_it == clients.end()) {
        std::cerr << "Erro: Cliente de origem ou destino não encontrado." << std::endl;
        return TransactionStatus::ERROR_CLIENT_NOT_FOUND;
    }
    if (source_it->getLastRequest() >= seqn) {
        std::cout << "Requisição duplicada #" << seqn << " de " << source_it->getAddress() << std::endl;
        return TransactionStatus::ERROR_DUPLICATE_REQUEST;
    }
    if (source_it->getBalance() < value) {
        std::cout << "Saldo insuficiente para o cliente " << source_it->getAddress() << std::endl;
        return TransactionStatus::ERROR_INSUFFICIENT_FUNDS;
    }
    return TransactionStatus::SUCCESS;
}

void Server::executeTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn) {
    source_it->setBalance(source_it->getBalance() - value);
    dest_it->setBalance(dest_it->getBalance() + value);
    source_it->setLastRequest(seqn);
}

void Server::updateAndLogTransaction(const std::string& source_ip, const std::string& dest_ip, int value, int seqn) {
    this->num_transactions++;
    this->total_transferred += value;

    std::cout << "Transação #" << seqn << ": " << source_ip << " -> " << dest_ip << " [Valor: " << value << "]" << std::endl;
}

float Server::processTransaction(const std::string& source_ip, uint32_t dest_addr_int, int value, int seqn) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    struct in_addr dest_ip_struct;
    dest_ip_struct.s_addr = dest_addr_int;
    std::string dest_ip = inet_ntoa(dest_ip_struct);

    std::vector<ClientDTO>::iterator source_it = findClient(source_ip);
    std::vector<ClientDTO>::iterator dest_it = findClient(dest_ip);

    TransactionStatus status = validateTransaction(source_it, dest_it, value, seqn);
    if (status != TransactionStatus::SUCCESS) {
        return (source_it != clients.end()) ? source_it->getBalance() : -1.0f;
    }

    executeTransaction(source_it, dest_it, value, seqn);

    updateAndLogTransaction(source_ip, dest_ip, value, seqn);

    printClients_unlocked();

    return source_it->getBalance();
}

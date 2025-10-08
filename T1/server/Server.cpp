
#include "Server.h"
#include "serverInterface.h"
#include "discovery.h"
#include "transactions.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <arpa/inet.h>

#define INITIAL_BALANCE 1000.0f

Server::Server(int port)
    : udp_port(port), num_transactions(0), total_transferred(0), total_balance(0), transaction_history(), clients(), server_socket(port) {}

int Server::getNumTransactions() const {return num_transactions; }
int Server::getTotalTransferred() const {return total_transferred; }
int Server::getTotalBalance() const {return total_balance; }

void Server::init(int port) {
    struct sockaddr_in client_addr;
    Packet received_packet;

    if (server_socket.createSocket() == false) {
        throw std::runtime_error("Erro ao criar o socket do servidor.");
    }

    server_socket.configureBroadcast();
    server_socket.setUpServerAddress();
    server_socket.bindSocket();

    logInitialMessage(num_transactions, static_cast<int>(total_transferred), static_cast<int>(total_balance));

    std::thread interface_thread(&Server::interfaceThread, this);
    interface_thread.detach();

    while (true) {
        int n = server_socket.receivePacket(received_packet, client_addr);
        if (n <= 0) continue;

        switch (ntohs(received_packet.type)) {
            case TRANSACTION_REQ: {
                Packet req_copy = received_packet;
                handleTransactionRequest(req_copy, client_addr, *this, server_socket);
                break;
            }
            case DISCOVERY: {
                handleDiscoveryPacket(server_socket, *this, client_addr);
                break;
            }
            default:
                std::cerr << "Tipo de pacote desconhecido recebido." << std::endl;
                break;
        }
    }

    server_socket.closeSocket();
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
    std::lock_guard<std::mutex> lock(lock_reader);
    printClients_unlocked();
}

bool Server::wasClientAdded(const std::string& client_ip) {
    std::vector<ClientDTO>::iterator it = std::find_if(clients.begin(), clients.end(), [&](const ClientDTO& c) {
        return c.getAddress() == client_ip;
    });

    if (it == clients.end()) {
        ClientDTO newClient(client_ip);
        clients.push_back(newClient);
        
        this->total_balance += newClient.getBalance();
        return true;
    }
    std::cout << "Cliente " << client_ip << " já está registado." << std::endl;
    return false;
}

std::vector<ClientDTO>::iterator Server::findClient(const std::string& ip) {
    return std::find_if(clients.begin(), clients.end(), [&](const ClientDTO& c) {
        return c.getAddress() == ip;
    });
}

void Server::addClient(const std::string& client_ip) {
    bool client_added = false;
    {
        std::unique_lock<std::mutex> lock(lock_reader);
        client_added = wasClientAdded(client_ip);
    }
    condition_reader.notify_all();

    if (client_added) {
        std::lock_guard<std::mutex> lock(lock_reader);
        data_changed = true;
        condition_reader.notify_all();
    }
}

TransactionStatus Server::validateTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn) {
    if (source_it == clients.end() || dest_it == clients.end()) {
        std::cerr << "Erro: Cliente de origem ou destino não encontrado." << std::endl;
        return TransactionStatus::ERROR_CLIENT_NOT_FOUND;
    }
    if (seqn <= source_it->getLastRequest()) {
        std::cout << "Requisição duplicada #" << seqn << " de " << source_it->getAddress() << std::endl;
        return TransactionStatus::ERROR_DUPLICATE_REQUEST;
    }
    if (seqn > source_it->getLastRequest() + 1) {
        std::cout << "Requisição fora de ordem #" << seqn << " de " << source_it->getAddress() 
                  << ". Esperado: " << source_it->getLastRequest() + 1 << std::endl;
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

    Transaction new_transaction;
    new_transaction.id = this->num_transactions;
    new_transaction.source_ip = source_ip;
    new_transaction.dest_ip = dest_ip;
    new_transaction.value = value;
    new_transaction.client_seqn = seqn;
    this->transaction_history.push_back(new_transaction);
}

std::pair<TransactionStatus, float> Server::processTransaction(const std::string& source_ip, uint32_t dest_addr_int, int value, int seqn) {
    struct in_addr dest_ip_struct;
    dest_ip_struct.s_addr = dest_addr_int;
    std::string dest_ip = inet_ntoa(dest_ip_struct);
    std::pair<TransactionStatus, float> result;
    bool transaction_processed = false;

    {
        std::unique_lock<std::mutex> lock(lock_reader);

        std::vector<ClientDTO>::iterator source_it = findClient(source_ip);
        std::vector<ClientDTO>::iterator dest_it = findClient(dest_ip);

        TransactionStatus status = validateTransaction(source_it, dest_it, value, seqn);

        if (status == TransactionStatus::SUCCESS) {
            executeTransaction(source_it, dest_it, value, seqn);
            updateAndLogTransaction(source_ip, dest_ip, value, seqn);
            last_log_info.type = LogType::SUCCESS;
            result = {TransactionStatus::SUCCESS, source_it->getBalance()};
            transaction_processed = true;
        } else if (status == TransactionStatus::ERROR_DUPLICATE_REQUEST) {
            last_log_info.type = LogType::DUPLICATE;
            last_log_info.transaction_id = seqn;
            last_log_info.value = value;
            last_log_info.source_ip = source_ip;
            last_log_info.dest_ip = inet_ntoa({dest_addr_int});
            transaction_processed = true;
        } else {
            result = {status, (source_it == clients.end()) ? -1.0f : source_it->getBalance()};
        }
    }

    condition_reader.notify_all();

    if (transaction_processed) {
        std::lock_guard<std::mutex> lock(lock_reader);
        data_changed = true;
        condition_reader.notify_all();
    }

    return result;
}

void Server::interfaceThread() {
    while (true) {
        std::unique_lock<std::mutex> lock(lock_reader);
        
        condition_reader.wait(lock, [this] { return data_changed; });

        lock.unlock();

        if (last_log_info.type == LogType::SUCCESS) {
            if (!transaction_history.empty()) {
                const auto& last_transaction = transaction_history.back();
                logRequisitionMessage(
                    last_transaction.source_ip, 
                    last_transaction.client_seqn, 
                    last_transaction.dest_ip, 
                    last_transaction.value,
                    num_transactions, 
                    total_transferred, 
                    total_balance
                );
            }
        } else if (last_log_info.type == LogType::DUPLICATE) {
            std::string log_msg = getDuplicatedMessage(
                last_log_info.source_ip, 
                last_log_info.transaction_id, 
                last_log_info.dest_ip, 
                last_log_info.value,
                num_transactions, 
                total_transferred, 
                total_balance
            );
            std::cout << log_msg << std::endl;
        }

        data_changed = false;
        last_log_info.type = LogType::NONE;
        
        lock.lock();
    }
}
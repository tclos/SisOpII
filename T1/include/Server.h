#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <string>
#include <mutex>
#include "ClientDTO.h"

enum class TransactionStatus {
    SUCCESS,
    ERROR_CLIENT_NOT_FOUND,
    ERROR_DUPLICATE_REQUEST,
    ERROR_INSUFFICIENT_FUNDS
};

class Server {
    private:
        int udp_port;
        int num_transactions;
        int total_transferred;
        int total_balance;
        std::vector<ClientDTO> clients;
        mutable std::mutex clients_mutex;
        void addClient_unlocked(const std::string& client_ip);
        void printClients_unlocked() const;
        std::vector<ClientDTO>::iterator findClient(const std::string& ip);
        TransactionStatus validateTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn);
        void executeTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn);
        void updateAndLogTransaction(const std::string& source_ip, const std::string& dest_ip, int value, int seqn);
    
    public:
        Server(int port);

        void addClient(const std::string& client_ip);
        void printClients() const;
        float processTransaction(const std::string& source_ip, uint32_t dest_addr, int value, int seqn);
        void init(int port);
};

#endif // SERVER_H

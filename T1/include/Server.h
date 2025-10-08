#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include "ClientDTO.h"
#include "ServerUDP.h"

enum class TransactionStatus {
    SUCCESS,
    ERROR_CLIENT_NOT_FOUND,
    ERROR_DUPLICATE_REQUEST,
    ERROR_INSUFFICIENT_FUNDS
};

struct Transaction {
    int id;
    std::string source_ip;
    std::string dest_ip;
    int value;
    int client_seqn;
};

class Server {
    private:
        int udp_port;
        int num_transactions;
        int total_transferred;
        int total_balance;
        ServerUDP server_socket;
        std::vector<ClientDTO> clients;
        std::vector<Transaction> transaction_history;

        mutable std::mutex lock_reader;
        mutable std::condition_variable condition_reader;
        mutable int active_readers = 0;
        bool writer_active = false;
        bool data_changed = false;

        void interfaceThread();
        
        bool wasClientAdded(const std::string& client_ip);
        void printClients_unlocked() const;
        std::vector<ClientDTO>::iterator findClient(const std::string& ip);
        TransactionStatus validateTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn);
        void executeTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn);
        void updateAndLogTransaction(const std::string& source_ip, const std::string& dest_ip, int value, int seqn);
    
    public:
        Server(int port);

        int getNumTransactions() const;
        int getTotalTransferred() const;
        int getTotalBalance() const;
        void addClient(const std::string& client_ip);
        void printClients() const;
        std::pair<TransactionStatus, float> processTransaction(const std::string& source_ip, uint32_t dest_addr, int value, int seqn);
        void init(int port);
};

#endif // SERVER_H

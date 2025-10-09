#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include "ClientDTO.h"
#include "ServerUDP.h"
#include "serverInterface.h"
#include "utils.h"

class Server {
    private:
        int udp_port;
        int num_transactions;
        int total_transferred;
        int total_balance;
        ServerUDP server_socket;
        std::vector<ClientDTO> clients;
        std::vector<Transaction> transaction_history;
        ServerInterface interface;

        mutable std::mutex data_mutex;

        LogInfo last_log_info;
        
        bool wasClientAdded(const std::string& client_ip);
        std::vector<ClientDTO>::iterator findClient(const std::string& ip);
        TransactionStatus validateTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn);
        void executeTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn);
        void updateAndLogTransaction(const std::string& source_ip, const std::string& dest_ip, int value, int seqn);
    
    public:
        Server(int port);

        int getNumTransactions() const;
        int getTotalTransferred() const;
        int getTotalBalance() const;
        LogInfo getLastLogInfo() const;
        Transaction getLastTransaction() const;

        void addClient(const std::string& client_ip);
        void printClients() const;
        std::pair<TransactionStatus, float> processTransaction(const std::string& source_ip, uint32_t dest_addr, int value, int seqn);
        void init(int port);
};

#endif // SERVER_H

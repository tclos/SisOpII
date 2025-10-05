#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <string>
#include <mutex>
#include "Client.h"

class Server {
    private:
        int udp_port;
        int num_transactions;
        int total_transferred;
        int total_balance;
        std::vector<Client> clients;
        mutable std::mutex clients_mutex;
    
    public:
        Server(int port);

        int getPort() const;
        void addClient(const std::string& client_ip, float balance);
        void printClients() const;
        int getNumTransactions() const;
        float getTotalTransferred() const;
        float getTotalBalance() const;

        void init(int port);
};

#endif // SERVER_H

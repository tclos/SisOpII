#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <string>
#include "Client.h"

class Server {
    private:
        int udp_port;
        int num_transactions;
        int total_transferred;
        int total_balance;
        std::vector<Client> clients;
    
    public:
        Server(int port);

        int getPort() const;
        int getNumTransactions() const;
        float getTotalTransferred() const;
        float getTotalBalance() const;

        void init(int port);
};

#endif // SERVER_H

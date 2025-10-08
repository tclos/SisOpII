#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "ClientUDP.h"

class Client {
    private:
        std::string server_address;
        std::string address;
        int last_req;
        float balance;
        int port;
        int sequence_number;
        struct sockaddr_in server_sock_addr;
        ClientUDP client_socket;
    
    public:
        Client(int port);
        Client(std::string addr, int port);

        std::string getAddress() const;
        int getLastRequest() const;
        float getBalance() const;

        void setLastRequest(int last_req);
        void setBalance(float bal);
        void init();
};

#endif // CLIENT_H

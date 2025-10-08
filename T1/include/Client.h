#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <condition_variable>
#include <queue>
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

        std::queue<std::string> command_queue;
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool finished = false;

        void userInputThread();
        void communicationThread();

        bool tryGetCommandFromQueue(std::string& out_command);
        void processCommand(const std::string& command);
        void executeRequestWithRetries(const std::string& dest_ip, int value);
    
    public:
        Client(int port);
        Client(std::string addr, int port);

        std::string getAddress() const;
        int getLastRequest() const;
        float getBalance() const;

        void setLastRequest(int last_req);
        void setBalance(float bal);
        void init();
        void shutdown();
};

#endif // CLIENT_H

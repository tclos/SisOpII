#ifndef CLIENTUDP_H
#define CLIENTUDP_H

#include <string>
#include <netinet/in.h>
#include <netdb.h>

class ClientUDP {
    private:
        std::string host;
        int port;
        int sockfd;
        struct sockaddr_in server_addr, client_addr;
        struct hostent *server;

        struct hostent* getServerByHost(const std::string& host);

    public:
        ClientUDP(const std::string& host, int port);
        bool setup();
        bool sendMessage(const std::string& message);
        bool receiveMessage();
        void closeSocket();
        void run();
        ~ClientUDP();
};

#endif // CLIENTUDP_H

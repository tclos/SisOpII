#ifndef CLIENTUDP_H
#define CLIENTUDP_H

#include "utils.h"
#include <string>
#include <netinet/in.h>
#include <netdb.h>

class ClientUDP {
    private:
        int port;
        int sockfd;
        struct sockaddr_in server_addr, client_addr;
        struct hostent *server;

    public:
        ClientUDP(int port);
        bool setupBroadcast();
        bool sendMessage(const std::string& message);
        bool sendPacket(const Packet& packet, const struct sockaddr_in& dest_addr);
        int receivePacket(Packet& packet, struct sockaddr_in& sender_addr);
        void closeSocket();
        ~ClientUDP();
};

#endif // CLIENTUDP_H

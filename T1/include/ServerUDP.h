#ifndef SERVERUDP_H
#define SERVERUDP_H

#include "utils.h"
#include <netinet/in.h>

class ServerUDP {
private:
    int sockfd;
    struct sockaddr_in server_addr;
    int port;

public:
    ServerUDP(int port);
    bool createSocket();
    void setUpServerAddress();
    bool bindSocket();
    void configureBroadcast();
    int receivePacket(Packet& packet, struct sockaddr_in& sender_addr);
    bool sendPacket(const Packet& packet, const struct sockaddr_in& dest_addr);
    void closeSocket();
    ~ServerUDP();
};

#endif // SERVERUDP_H

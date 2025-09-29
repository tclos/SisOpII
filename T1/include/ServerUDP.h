#ifndef SERVERUDP_H
#define SERVERUDP_H

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
    void receiveAndRespond();
    void closeSocket();
    void run();
    ~ServerUDP();
};

#endif // SERVERUDP_H

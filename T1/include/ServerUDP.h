#ifndef SERVERUDP_H
#define SERVERUDP_H

#include "discovery.h"
#include <netinet/in.h>
#include <vector>
#include <string>
#include <mutex>
#include "Client.h"


class ServerUDP {
private:
    int sockfd;
    struct sockaddr_in server_addr;
    int port;

    std::vector<Client> clients;
    std::mutex mtx;

    Client* findClientByAddr(const std::string& addr);
    bool transferValue(Client* source, const std::string& destAddr, int value);
    void sendAck(const sockaddr_in& client_addr, int seqn, float newBalance);

public:
    ServerUDP(int port);
    bool createSocket();
    void setUpServerAddress();
    bool bindSocket();
    void configureBroadcast();
    void receiveAndRespond();
    int receivePacket(Packet& packet, struct sockaddr_in& sender_addr);
    bool sendPacket(const Packet& packet, const struct sockaddr_in& dest_addr);
    void closeSocket();
    void run();
    ~ServerUDP();
};

#endif // SERVERUDP_H

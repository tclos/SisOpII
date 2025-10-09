#include "ServerUDP.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

ServerUDP::ServerUDP(int port) : sockfd(-1), port(port) {}

bool ServerUDP::createSocket() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcastEnable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    if (sockfd < 0) {
        std::cerr << "Erro ao criar socket" << std::endl;
        return false;
    }
    return true;
}

void ServerUDP::configureBroadcast() {
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        std::cerr << "Erro ao configurar opção de broadcast" << std::endl;
        close(sockfd);
    }
}

void ServerUDP::setUpServerAddress() {
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
}

bool ServerUDP::bindSocket() {
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Erro ao fazer bind do socket" << std::endl;
        close(sockfd);
        return false;
    }
    return true;
}

int ServerUDP::receivePacket(Packet& packet, struct sockaddr_in& sender_addr) {
    socklen_t len = sizeof(sender_addr);
    int n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&sender_addr, &len);
    if (n < 0) {
        std::cerr << "Erro ao receber dados" << std::endl;
    }
    return n;
}

bool ServerUDP::sendPacket(const Packet& packet, const struct sockaddr_in& dest_addr) {
    if (sendto(sockfd, &packet, sizeof(packet), 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        std::cerr << "Erro ao enviar dados" << std::endl;
        return false;
    }
    return true;
}

bool ServerUDP::sendMessage(const std::string& message, const struct sockaddr_in& dest_addr) {
    if (sendto(sockfd, message.c_str(), message.length(), 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        std::cerr << "Erro ao enviar mensagem de texto" << std::endl;
        return false;
    }
    return true;
}

void ServerUDP::closeSocket() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

ServerUDP::~ServerUDP() {
    closeSocket();
}

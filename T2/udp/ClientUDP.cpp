#include "ClientUDP.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/time.h>

ClientUDP::ClientUDP(int port)
    : sockfd(-1) {
    // CORRETO: dentro do corpo do construtor
    (void)port;  // Para silenciar warning de parâmetro não usado
}

bool ClientUDP::createSocket() {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Erro ao criar o socket do cliente." << std::endl;
        return false;
    }
    return true;
}

bool ClientUDP::setupBroadcast() {
    if (createSocket() == false) {
        return false;
    }

    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        close(sockfd);
        std::cerr << "Erro ao configurar a opção de broadcast." << std::endl;
        return false;
    }

    return true;
}

void ClientUDP::setReceiveTimeout(int seconds, int microseconds) {
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = microseconds;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Erro ao configurar o timeout de recebimento." << std::endl;
    }
}

bool ClientUDP::sendPacket(const Packet& packet, const struct sockaddr_in& dest_addr) {
    if (sendto(sockfd, &packet, sizeof(packet), 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        return false;
    }
    return true;
}

int ClientUDP::receivePacket(Packet& packet, struct sockaddr_in& sender_addr) {
    socklen_t len = sizeof(sender_addr);
    int n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&sender_addr, &len);
    return n;
}

bool ClientUDP::sendMessage(const std::string& message) {
    unsigned int len = sizeof(server_addr);
    int n = sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&server_addr, len);
    if (n < 0) {
        std::cerr << "Erro ao enviar mensagem" << std::endl;
        return false;
    }
    std::cout << "Mensagem enviada: " << message << std::endl;
    return true;
}

void ClientUDP::closeSocket() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

ClientUDP::~ClientUDP() {
    closeSocket();
}
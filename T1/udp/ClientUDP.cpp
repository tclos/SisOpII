#include "ClientUDP.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

ClientUDP::ClientUDP(const std::string& host, int port)
    : host(host), port(port), sockfd(-1), server(nullptr) {}

struct hostent* ClientUDP::getServerByHost(const std::string& host) {
    struct hostent* server = gethostbyname(host.c_str());
    if (server == NULL) {
        std::cerr << "Erro: Host desconhecido " << host << std::endl;
    }
    return server;
}

bool ClientUDP::setup() {
    server = getServerByHost(host);
    if (server == NULL) {
        return false;
    }
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Erro ao abrir socket" << std::endl;
        return false;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    std::memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
    return true;
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

bool ClientUDP::receiveMessage() {
    char buffer[1024];
    unsigned int len = sizeof(client_addr);
    int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &len);
    if (n < 0) {
        std::cerr << "Erro ao receber mensagem" << std::endl;
        return false;
    }
    buffer[n] = '\0';
    std::cout << "Mensagem recebida: " << buffer << std::endl;
    return true;
}

void ClientUDP::closeSocket() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

void ClientUDP::run() {
    if (!setup()) return;
    if (!sendMessage("Hello, UDP Server!")) {
        closeSocket();
        return;
    }
    receiveMessage();
    closeSocket();
}

ClientUDP::~ClientUDP() {
    closeSocket();
}

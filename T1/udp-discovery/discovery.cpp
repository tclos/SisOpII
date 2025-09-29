#include "discovery.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>

void run_discovery_service_server(int server_port) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    Packet packet;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw std::runtime_error("Erro ao criar o socket do servidor.");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Erro no bind do servidor.");
    }

    std::cout << "Servidor de descoberta escutando na porta " << server_port << "..." << std::endl;

    while (true) {
        int n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &client_len);
        if (n > 0 && ntohs(packet.type) == DISCOVERY) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << "Mensagem de DESCOBERTA recebida de " << client_ip << std::endl;

            // Responde ao cliente (unicast)
            packet.type = htons(DISCOVERY_ACK);
            sendto(sockfd, &packet, sizeof(packet), 0, (const struct sockaddr *)&client_addr, client_len);
        }
    }

    close(sockfd);
}

std::string run_discovery_service_client(int server_port) {
    int sockfd;
    struct sockaddr_in broadcast_addr, server_addr;
    socklen_t server_len = sizeof(server_addr);
    Packet discovery_packet, response_packet;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw std::runtime_error("Erro ao criar o socket do cliente.");
    }

    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        close(sockfd);
        throw std::runtime_error("Erro ao configurar a opção de broadcast.");
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(server_port);
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255"); // Endereço de broadcast

    discovery_packet.type = htons(DISCOVERY);

    std::cout << "Enviando mensagem de DESCOBERTA em broadcast..." << std::endl;
    if (sendto(sockfd, &discovery_packet, sizeof(discovery_packet), 0, (const struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
        close(sockfd);
        throw std::runtime_error("Erro ao enviar a mensagem de descoberta.");
    }

    // Aguarda a resposta do servidor
    int n = recvfrom(sockfd, &response_packet, sizeof(response_packet), 0, (struct sockaddr *)&server_addr, &server_len);
    if (n > 0 && ntohs(response_packet.type) == DISCOVERY_ACK) {
        char server_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
        close(sockfd);
        return std::string(server_ip);
    }

    close(sockfd);
    throw std::runtime_error("Nenhuma resposta de descoberta recebida do servidor.");
}
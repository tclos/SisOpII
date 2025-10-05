#include "discovery.h"
#include "ClientUDP.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>

#define BROADCAST_ADDR "255.255.255.255"

std::string run_discovery_service_client(int server_port) {
    struct sockaddr_in broadcast_addr, server_addr;
    Packet discovery_packet, response_packet;

    ClientUDP client(server_port);

    if (client.setupBroadcast() == false) {
        throw std::runtime_error("Erro ao configurar o socket do cliente para broadcast.");
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(server_port);
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);

    discovery_packet.type = htons(DISCOVERY);

    if (!client.sendPacket(discovery_packet, broadcast_addr)) {
        throw std::runtime_error("Erro ao enviar a mensagem de descoberta.");
        client.closeSocket();
    }

    int n = client.receivePacket(response_packet, server_addr);
    if (n > 0 && ntohs(response_packet.type) == DISCOVERY_ACK) {
        char server_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
        client.closeSocket();
        return std::string(server_ip);
    }

    client.closeSocket();
    throw std::runtime_error("Nenhuma resposta de descoberta recebida do servidor.");
}

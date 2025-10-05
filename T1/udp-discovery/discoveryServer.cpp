#include "discovery.h"
#include "ServerUDP.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>

#define INITIAL_BALANCE 1000.0f

void run_discovery_service_server(int server_port, Server& server_data) {
    struct sockaddr_in client_addr;
    Packet received_packet, response_packet;

    ServerUDP server(server_port);

    if (server.createSocket() == false) {
        throw std::runtime_error("Erro ao criar o socket do servidor.");
    }

    server.configureBroadcast();
    server.setUpServerAddress();
    server.bindSocket();

    while (true) {
        int n = server.receivePacket(received_packet, client_addr);

        if (n > 0 && ntohs(received_packet.type) == DISCOVERY) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

            server_data.addClient(client_ip, INITIAL_BALANCE);

            response_packet.type = htons(DISCOVERY_ACK);
            server.sendPacket(response_packet, client_addr);
        }
    }

    server.closeSocket();
}

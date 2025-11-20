#include "discovery.h"
#include "ServerUDP.h"
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

void handleDiscoveryPacket(ServerUDP& server_socket, Server& server_data, struct sockaddr_in& client_addr) {

    if(server_data.getRole() != PRIMARY) return;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    server_data.addClient(client_ip);

    Packet response_packet;
    response_packet.type = htons(DISCOVERY_ACK);
    server_socket.sendPacket(response_packet, client_addr);

    server_data.printClients();
}

std::string run_discovery_service_server(ServerUDP& server_socket, int port) {
    std::cout << "Procurando servidor Primário na rede" << std::endl;

    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    inet_pton(AF_INET, BROADCAST_ADDR, &broadcast_addr.sin_addr);

    Packet discovery_packet;
    discovery_packet.type = htons(SERVER_DISCOVERY);
    
    for(int i = 0; i < 3; ++i) {
        server_socket.sendPacket(discovery_packet, broadcast_addr);
    }

    server_socket.setReceiveTimeout(2, 0);

    Packet response_packet;
    struct sockaddr_in sender_addr;
    
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count() > 2) {
            break;
        }

        int n = server_socket.receivePacket(response_packet, sender_addr);
        
        if (n > 0) {
            if (ntohs(response_packet.type) == DISCOVERY_ACK) {
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &sender_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
                
                server_socket.setReceiveTimeout(0, 0);
                return std::string(ip_str);
            }
        } else {
            break;
        }
    }

    server_socket.setReceiveTimeout(0, 0);
    return "";
}
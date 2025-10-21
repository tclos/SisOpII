#include "discovery.h"
#include "ServerUDP.h"
#include <arpa/inet.h>

void handleDiscoveryPacket(ServerUDP& server_socket, Server& server_data, struct sockaddr_in& client_addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    server_data.addClient(client_ip);

    Packet response_packet;
    response_packet.type = htons(DISCOVERY_ACK);
    server_socket.sendPacket(response_packet, client_addr);

    server_data.printClients();
}

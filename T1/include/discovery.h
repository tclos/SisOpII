#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>
#include <cstdint>
#include "Server.h"

enum PacketType {
    DISCOVERY = 0,
    DISCOVERY_ACK = 1
};

struct Packet {
    uint16_t type; // Tipo do pacote (e.g., DESCOBERTA)
};

void run_discovery_service_server(int server_port, Server& server_data);

std::string run_discovery_service_client(int server_port);

#endif // DISCOVERY_H

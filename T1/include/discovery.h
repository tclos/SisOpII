#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>
#include <cstdint>
#include "Server.h"
#include "ServerUDP.h"

void handleDiscoveryPacket(ServerUDP& server_socket, Server& server_data, struct sockaddr_in& client_addr);

std::string run_discovery_service_client(int server_port);

#endif // DISCOVERY_H

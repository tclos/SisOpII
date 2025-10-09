#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include <string>
#include <cstdint>
#include "Server.h"
#include "ServerUDP.h"
#include "ClientUDP.h"
#include "utils.h"

void handleTransactionRequest(const Packet& request_packet, const struct sockaddr_in& client_addr, Server& server_data, ServerUDP& server_socket);
bool validateInput(const std::string& line, std::string& dest_ip, int& value);
bool sendRequestPacket(int sequence_number, const std::string& dest_ip, int value, ClientUDP& client_socket, const sockaddr_in& server_addr);
void logResponse(const Packet& response, int seqn, const std::string& server_ip, const std::string& dest_ip, int value);
bool receiveResponse(Packet& response_packet, int sequence_number, ClientUDP& client_socket);

#endif // TRANSACTIONS_H

#include "Client.h"
#include "discovery.h"
#include "transactions.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

#define TIMEOUT_MS 10000 // 10 milissegundos
#define MAX_RETRIES 5

Client::Client(int port)
    : port(port), sequence_number(1), client_socket(port) {
    this->server_address = "";
    memset(&server_sock_addr, 0, sizeof(server_sock_addr));
}

int Client::getSequenceNumber() const { return sequence_number; }
std::string Client::getServerAddress() const { return server_address; }

std::string Client::discoverServer() {
    this->server_address = run_discovery_service_client(port);
    
    client_socket.createSocket();
    client_socket.setReceiveTimeout(0, TIMEOUT_MS);

    server_sock_addr.sin_family = AF_INET;
    server_sock_addr.sin_port = htons(this->port);
    inet_pton(AF_INET, this->server_address.c_str(), &server_sock_addr.sin_addr);

    return this->server_address;
}

std::pair<bool, AckData> Client::executeRequestWithRetries(const std::string& dest_ip, int value) {
    int retries = 0;
    bool ack_received = false;

    while (!ack_received && retries < MAX_RETRIES) {
        if (!sendRequestPacket(sequence_number, dest_ip, value, client_socket, server_sock_addr)) {
            return {false, {}};
        }

        Packet response_packet;
        if (receiveResponse(response_packet, sequence_number, client_socket)) {
            sequence_number++;
            return {true, response_packet.data.ack};
        } else {
            retries++;
        }
    }
    
    sequence_number++; 
    return {false, {}};
}
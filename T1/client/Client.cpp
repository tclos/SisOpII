#include "Client.h"
#include "utils.h"
#include "clientInterface.h"
#include "discovery.h"
#include "transactions.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

Client::Client(int port)
    : port(port), sequence_number(1), client_socket(port) {
    this->server_address = "";
    memset(&server_sock_addr, 0, sizeof(server_sock_addr));
}

Client::Client(std::string addr, int port)
    : address(addr),
      server_address(""),
      last_req(0),
      balance(INITIAL_BALANCE),
      port(port),
      sequence_number(1),
      client_socket(port)
{
    memset(&server_sock_addr, 0, sizeof(server_sock_addr));
} 

std::string Client::getAddress() const { return address; }
int Client::getLastRequest() const { return last_req; }
float Client::getBalance() const { return balance; }

void Client::setLastRequest(int last_req) { this->last_req = last_req; }
void Client::setBalance(float bal) { balance = bal; }

void Client::init() {
    this->server_address = run_discovery_service_client(port);
    logInitialMessage(this->server_address);

    if (!client_socket.setupBroadcast()) {
        throw std::runtime_error("Erro ao criar o socket do cliente para transações.");
    }

    server_sock_addr.sin_family = AF_INET;
    server_sock_addr.sin_port = htons(this->port);
    inet_pton(AF_INET, this->server_address.c_str(), &server_sock_addr.sin_addr);

    for (std::string line; std::getline(std::cin, line);) {
        sendRequest(line, this->sequence_number, this->client_socket, this->server_sock_addr, this->server_address);
    }
}

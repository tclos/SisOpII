#include "ServerUDP.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <cstring> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>

ServerUDP::ServerUDP(int port) : sockfd(-1), port(port) {}

bool ServerUDP::createSocket() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcastEnable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    if (sockfd < 0) {
        std::cerr << "Erro ao criar socket" << std::endl;
        return false;
    }
    return true;
}

void ServerUDP::configureBroadcast() {
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        std::cerr << "Erro ao configurar opção de broadcast" << std::endl;
        close(sockfd);
    }
}

void ServerUDP::setUpServerAddress() {
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
}

bool ServerUDP::bindSocket() {
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Erro ao fazer bind do socket" << std::endl;
        close(sockfd);
        return false;
    }
    return true;
}

int ServerUDP::receivePacket(Packet& packet, struct sockaddr_in& sender_addr) {
    socklen_t len = sizeof(sender_addr);
    int n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&sender_addr, &len);
    if (n < 0) {
        std::cerr << "Erro ao receber dados" << std::endl;
    }
    return n;
}

bool ServerUDP::sendPacket(const Packet& packet, const struct sockaddr_in& dest_addr) {
    if (sendto(sockfd, &packet, sizeof(packet), 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        std::cerr << "Erro ao enviar dados" << std::endl;
        return false;
    }
    return true;
}

void ServerUDP::receiveAndRespond() {
    char buffer[1024];
    int clientLen = sizeof(server_addr);
    int n = recvfrom(sockfd, (char *)buffer, 1024, 0, (struct sockaddr *)&server_addr, (socklen_t *)&clientLen);

    if (n < 0) {
        std::cerr << "Erro ao receber dados" << std::endl;
    } else {
        buffer[n] = '\0';
        std::cout << "Mensagem recebida: " << buffer << std::endl;

        n = sendto(sockfd, (const char *)buffer, n, 0, (const struct sockaddr *)&server_addr, clientLen);
        if (n < 0) {
            std::cerr << "Erro ao enviar dados" << std::endl;
        } else {
            std::cout << "Mensagem enviada de volta ao cliente" << std::endl;
        }
    }
}

// Searches for a client in the server's client list according to its address
Client* ServerUDP::findClientByAddr(const std::string& addr) {
    // Iterate over all clients stored in the server
    for (auto &client : clients) {
        // Check if the client address matches the requested one
        if (client.getAddress() == addr) {
            return &client; // pointer to client
        }
    }
    return nullptr; // client not found
}

// Transfers a specified value from one client to another
bool ServerUDP::transferValue(Client* source, const std::string& destAddr, int value) {
    Client* dest = findClientByAddr(destAddr);
    if (!dest) return false; // destination not found

    if (source->getBalance() < value) return false; // insufficient funds

    // subtract value from source balance
    source->setBalance(source->getBalance() - value);
    // Add value to destination balance
    dest->setBalance(dest->getBalance() + value);

    // update server total
    total_transferred += value;

    return true;
}

// Sends an acknowledgment (ACK) message to the client
void ServerUDP::sendAck(const sockaddr_in& client_addr, int seqn, float updated_balance) {
    char buffer[1024];
    // ACK message format: "sequence_number updated_balance"
    sprintf(buffer, "%d %f", seqn, updated_balance);
    // Send message to client
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
}

void ServerUDP::receiveAndRespond() {
    char buffer[1024]; // Buffer for arriving UDP message

    sockaddr_in client_addr; 
    int clientLen = sizeof(client_addr); // Length of client address structure

    // Receive a message from a client
    int n = recvfrom(sockfd, (char *)buffer, 1024, 0, (struct sockaddr *)&client_addr, (socklen_t *)&clientLen);

    // Check for errors
    if (n < 0) {
        std::cerr << "Erro ao receber dados" << std::endl;
        return;
    }

    buffer[n] = '\0';

    // New thread for each request
    std::thread([this, buffer, n, client_addr, clientLen]() {
        // parsing request data (sequence number, destination, amount)
        int seqn, value;    // Sequence number and transfer amount
        char destIP[INET_ADDRSTRLEN]; // Dest. client IP address
        sscanf(buffer, "%d %s %d", &seqn, destIP, &value);

        // Convert the client address from sockaddr_in to string IP
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), clientIP, INET_ADDRSTRLEN);

        Client* client = findClientByAddr(clientIP);
        if (!client) return; // ignore unknwn clients

        // Locking server's data
        std::lock_guard<std::mutex> lock(mtx);

        // Last processed request from the client
        int last_req = client->getLastRequest();

        // Check if sequence number is the expected one
        if (seqn == last_req + 1){
            client->setLastRequest(seqn);
            transferValue(client, std::string(destIP), value); //update balances
            sendAck(client_addr, seqn, client->getBalance());
        }
        else if (seqn <= last_req) {
            // Duplicado (already processed)
            sendAck(client_addr, last_req, client->getBalance());
        }
        else {
            // missing previous request
            sendAck(client_addr, last_req, client->getBalance());
        }
    }).detach(); // Thread runs independently
}


void ServerUDP::closeSocket() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

void ServerUDP::run() {
    if (!createSocket()) return;
    setUpServerAddress();
    if (!bindSocket()) return;

    while (true) {
        receiveAndRespond();
    }
}

ServerUDP::~ServerUDP() {
    closeSocket();
}

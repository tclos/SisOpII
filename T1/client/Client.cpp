#include "Client.h"
#include "utils.h"
#include "clientInterface.h"
#include "discovery.h"
#include "transactions.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <thread>

#define TIMEOUT_MS 10000 // 10 seconds

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

void Client::userInputThread() {
    for (std::string line; std::getline(std::cin, line);) {
        if (line.empty()) continue;

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            command_queue.push(line);
        }
        condition.notify_one(); // Notifica a networkThread que um novo comando chegou
    }

    // (CTRL+D) sinaliza para a outra thread encerrar
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        finished = true;
    }
    condition.notify_one();
}

bool Client::tryGetCommandFromQueue(std::string& out_command) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    condition.wait(lock, [this] { return !command_queue.empty() || finished; });

    if (finished && command_queue.empty()) {
        return false;
    }

    out_command = command_queue.front();
    command_queue.pop();
    return true;
}

void Client::executeRequestWithRetries(const std::string& dest_ip, int value) {
    const int MAX_RETRIES = 5;
    int retries = 0;
    bool ack_received = false;

    while (!ack_received && retries < MAX_RETRIES) {
        if (!sendRequestPacket(sequence_number, dest_ip, value, client_socket, server_sock_addr)) {
            std::cerr << "Erro crítico ao tentar enviar pacote. Abortando requisição." << std::endl;
            break; 
        }

        Packet response_packet;
        if (receiveResponse(response_packet, sequence_number, client_socket)) {
            logResponse(response_packet, sequence_number, server_address, dest_ip, value);
            ack_received = true;
        } else {
            retries++;
            std::cout << "Timeout! Reenviando requisição #" << sequence_number 
                      << " (" << retries << "/" << MAX_RETRIES << ")" << std::endl;
        }
    }

    if (!ack_received) {
        std::cerr << "Não foi possível obter resposta do servidor para a requisição #" << sequence_number << "." << std::endl;
    }
}

void Client::processCommand(const std::string& command) {
    std::string dest_ip_str;
    int value;
    if (!validateInput(command, dest_ip_str, value)) {
        return;
    }

    executeRequestWithRetries(dest_ip_str, value);

    sequence_number++;
}

void Client::communicationThread() {
    std::string command;
    while (tryGetCommandFromQueue(command)) {
        processCommand(command);
    }
}


void Client::init() {
    this->server_address = run_discovery_service_client(port);
    logInitialMessage(this->server_address);

    client_socket.createSocket();
    client_socket.setReceiveTimeout(0, TIMEOUT_MS);

    server_sock_addr.sin_family = AF_INET;
    server_sock_addr.sin_port = htons(this->port);
    inet_pton(AF_INET, this->server_address.c_str(), &server_sock_addr.sin_addr);

    std::thread input_thread(&Client::userInputThread, this);
    std::thread communication_thread(&Client::communicationThread, this);

    input_thread.join();
    communication_thread.join();

    client_socket.closeSocket();
}

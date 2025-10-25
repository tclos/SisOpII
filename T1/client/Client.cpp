#include "Client.h"
#include "discovery.h"
#include "transactions.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

//esse arquivo gerencia o estado do cliente (como o ID da requisição atual) e lida com a lógica de rede (enviar, receber e tentar de novo)

#define TIMEOUT_MS 10000 // 10 milissegundos
#define MAX_RETRIES 5

Client::Client(int port) //Inicializa o cliente e define o ID da requisição como 1
    : port(port), sequence_number(1), client_socket(port) {
    this->server_address = "";
    memset(&server_sock_addr, 0, sizeof(server_sock_addr));
}

int Client::getSequenceNumber() const { return sequence_number; }
std::string Client::getServerAddress() const { return server_address; }

//chama a função que envia uma mensagem para a rede e espera uma resposta do servidor, 
//armazena o IP do servidor que respondeu, configura o socker principal do cliente com um timeout
std::string Client::discoverServer() { 
    this->server_address = run_discovery_service_client(port);
    
    client_socket.createSocket();
    client_socket.setReceiveTimeout(0, TIMEOUT_MS);

    server_sock_addr.sin_family = AF_INET;
    server_sock_addr.sin_port = htons(this->port);
    inet_pton(AF_INET, this->server_address.c_str(), &server_sock_addr.sin_addr);

    return this->server_address;
}

//entra em um loop whie que tenta no max MAX_RETRIES
//em cada tentativa chama o sendRequestPacket para enviar a transação
//depois chama receiveResponse para esperar por um ACK, se receber um ack válido a função retorna TRUE, se der timeout retorna FALSE
std::pair<bool, AckData> Client::executeRequestWithRetries(const std::string& dest_ip, int value) {
    int retries = 0;
    bool ack_received = false;

    while (!ack_received && retries < MAX_RETRIES) {
        if (!sendRequestPacket(sequence_number, dest_ip, value, client_socket, server_sock_addr)) {
            return {false, {}};
        }

        Packet response_packet;
        if (receiveResponse(response_packet, sequence_number, client_socket)) {
            return {true, response_packet.data.ack};
        } else {
            retries++;
        }
    }
    
    return {false, {}};
}

//incrementa o ID da requisição para a próxima transação
void Client::incrementSequenceNumber() {
    this->sequence_number++;
}

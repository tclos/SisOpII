#include "Client.h"
#include "discovery.h"
#include "transactions.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

//esse arquivo gerencia o estado do cliente (como o ID da requisição atual) e lida com a lógica de rede (enviar, receber e tentar de novo)
#define MAX_RETRIES 5

Client::Client(int port) //Inicializa o cliente e define o ID da requisição como 1
    : port(port), sequence_number(1), client_socket(0) {
    this->server_address = "";
    memset(&server_sock_addr, 0, sizeof(server_sock_addr));

    this->running = true;
    announcement_thread = std::thread(&Client::listenForAnnouncements, this);
}

int Client::getSequenceNumber() const { return sequence_number; }
std::string Client::getServerAddress() const { 
    std::lock_guard<std::mutex> lock(server_mutex);
    return server_address;
}

//chama a função que envia uma mensagem para a rede e espera uma resposta do servidor, 
//armazena o IP do servidor que respondeu, configura o socker principal do cliente com um timeout
std::string Client::discoverServer() { 
    std::string found_ip = run_discovery_service_client(port);
    
    client_socket.createSocket();
    client_socket.setReceiveTimeout(0, TIMEOUT_MS);

    {
        std::lock_guard<std::mutex> lock(server_mutex);
        this->server_address = found_ip;

        server_sock_addr.sin_family = AF_INET;
        server_sock_addr.sin_port = htons(this->port);
        inet_pton(AF_INET, this->server_address.c_str(), &server_sock_addr.sin_addr);
    }

    return found_ip;
}

//entra em um loop whie que tenta no max MAX_RETRIES
//em cada tentativa chama o sendRequestPacket para enviar a transação
//depois chama receiveResponse para esperar por um ACK, se receber um ack válido a função retorna TRUE, se der timeout retorna FALSE
std::pair<bool, AckData> Client::executeRequestWithRetries(const std::string& dest_ip, int value) {
    int retries = 0;
    bool ack_received = false;

    while (!ack_received && retries < MAX_RETRIES) {
        struct sockaddr_in current_server_addr;
        {
            std::lock_guard<std::mutex> lock(server_mutex);
            current_server_addr = server_sock_addr;
        }

        if (!sendRequestPacket(sequence_number, dest_ip, value, client_socket, current_server_addr)) {
            std::cerr << "Aviso: Falha no envio do pacote." << std::endl;
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

Client::~Client() {
    running = false;
    if (announcement_thread.joinable()) {
        announcement_thread.detach();
    }
}

void Client::listenForAnnouncements() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt falhou no listener do cliente");
        close(sockfd);
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind falhou no listener do cliente");
        close(sockfd);
        return;
    }

    Packet packet;
    struct sockaddr_in sender_addr;
    socklen_t len = sizeof(sender_addr);

    while (running) {
        int n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&sender_addr, &len);
        
        if (n > 0) {
            if (ntohs(packet.type) == NEW_PRIMARY_ANNOUNCEMENT) {
                char new_ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &sender_addr.sin_addr, new_ip_str, INET_ADDRSTRLEN);
                std::string new_ip(new_ip_str);

                std::lock_guard<std::mutex> lock(server_mutex);
                
                if (new_ip != this->server_address) {
                    std::cout << "Novo Servidor Primário anunciado: " << new_ip << std::endl;
                    
                    this->server_address = new_ip;
                    
                    memset(&server_sock_addr, 0, sizeof(server_sock_addr));
                    server_sock_addr.sin_family = AF_INET;
                    server_sock_addr.sin_port = htons(this->port);
                    inet_pton(AF_INET, this->server_address.c_str(), &server_sock_addr.sin_addr);
                }
            }
        }
    }
    close(sockfd);
}

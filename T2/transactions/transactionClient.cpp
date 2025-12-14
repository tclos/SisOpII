#include "transactions.h"
#include <iostream>
#include <string>
#include <sstream>
#include <arpa/inet.h>
#include "ClientUDP.h"
#include "utils.h"  // CORREÇÃO: Adicionada

bool validateInput(const std::string& line, std::string& dest_ip, int& value) {
    std::stringstream ss(line);
    if (!(ss >> dest_ip >> value)) {
        std::cerr << "Formato de comando inválido. Use: IP_DESTINO VALOR" << std::endl;
        return false;
    }
    return true;
}

bool sendRequestPacket(int sequence_number, const std::string& dest_ip, int value, ClientUDP& client_socket, const sockaddr_in& server_addr) {
    Packet request_packet;
    request_packet.type = htons(TRANSACTION_REQ);
    request_packet.data.req.value = htonl(value);
    request_packet.data.req.seqn = htonl(sequence_number);
    inet_pton(AF_INET, dest_ip.c_str(), &request_packet.data.req.dest_addr);

    if (!client_socket.sendPacket(request_packet, server_addr)) {
        std::cerr << "Erro ao enviar requisição para o servidor." << std::endl;
        return false;
    }
    return true;
}

void logResponse(const Packet& response, int seqn, const std::string& server_ip, const std::string& dest_ip, int value) {
    std::cout << getCurrentFormattedTime()
              << " server " << server_ip
              << " id_req " << seqn
              << " dest " << dest_ip
              << " value " << value
              << " new_balance " << response.data.ack.new_balance
              << std::endl;
}

bool receiveResponse(Packet& response_packet, int sequence_number, ClientUDP& client_socket) {
    struct sockaddr_in response_addr;
    int n = client_socket.receivePacket(response_packet, response_addr);
    
    if (n > 0 && ntohs(response_packet.type) == TRANSACTION_ACK && ntohl(response_packet.data.ack.seqn) == static_cast<uint32_t>(sequence_number)) {
        return true;
    }

    std::cerr << "Resposta inválida ou timeout do servidor para a requisição #" << sequence_number << std::endl;
    return false;
}
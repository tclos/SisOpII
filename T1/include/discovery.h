#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>
#include <cstdint>

enum PacketType {
    DISCOVERY = 0,
    DISCOVERY_ACK = 1
};

struct Packet {
    uint16_t type; // Tipo do pacote (e.g., DESCOBERTA)
};

/**
 * @brief Operação passiva do subserviço de descoberta (lado do Servidor).
 * Aguarda por mensagens de descoberta em broadcast e responde em unicast.
 *
 * @param server_port A porta na qual o servidor irá escutar por mensagens de descoberta.
 */
void run_discovery_service_server(int server_port);

/**
 * @brief Operação ativa do subserviço de descoberta (lado do Cliente).
 * Envia uma mensagem de descoberta em broadcast e aguarda a resposta do servidor.
 *
 * @param server_port A porta para a qual a mensagem de broadcast será enviada.
 * @return std::string O endereço IPv4 do servidor descoberto.
 */
std::string run_discovery_service_client(int server_port);

#endif // DISCOVERY_H
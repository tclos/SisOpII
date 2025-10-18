#include "transactions.h"
#include <thread>
#include <arpa/inet.h>

void handleTransactionRequest(const Packet& request_packet, const struct sockaddr_in& client_addr, Server& server_data, ServerUDP& server_socket) {
    std::thread([=, &server_data, &server_socket]() {
        char client_ip_cstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_cstr, INET_ADDRSTRLEN);
        std::string source_ip(client_ip_cstr);

        int value = ntohl(request_packet.data.req.value);
        int seqn = ntohl(request_packet.data.req.seqn);

        std::tuple<TransactionStatus, float, int> result = server_data.processTransaction(source_ip, request_packet.data.req.dest_addr, value, seqn);

        TransactionStatus status = std::get<0>(result);
        float balance = std::get<1>(result);
        int last_req = std::get<2>(result);

        Packet response_packet;
        response_packet.type = htons(TRANSACTION_ACK);
        response_packet.data.ack.seqn = htonl(seqn);
        response_packet.data.ack.status = htonl(static_cast<uint32_t>(status));
        response_packet.data.ack.new_balance = balance;

        response_packet.data.ack.last_req = htonl(last_req);
        
        server_socket.sendPacket(response_packet, client_addr);

    }).detach();
}
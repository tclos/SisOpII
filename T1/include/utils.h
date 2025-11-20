#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <netinet/in.h>

#define INITIAL_BALANCE 1000.0f
#define BROADCAST_ADDR "255.255.255.255"
#define TIMEOUT_MS 10000 // 10 milissegundos

enum class LogType {
    NONE,
    SUCCESS,
    DUPLICATE
};

enum ServerRole { PRIMARY, BACKUP };

struct LogInfo {
    LogType type = LogType::NONE;
    int transaction_id = 0;
    int value = 0;
    std::string source_ip;
    std::string dest_ip;
};

enum class TransactionStatus {
    SUCCESS,
    ERROR_CLIENT_NOT_FOUND,
    ERROR_DUPLICATE_REQUEST,
    ERROR_INSUFFICIENT_FUNDS,
    ERROR_OUT_OF_SEQUENCE
};

struct Transaction {
    int id;
    char source_ip[INET_ADDRSTRLEN];
    char dest_ip[INET_ADDRSTRLEN];
    int value;
    int client_seqn;
};

enum PacketType {
    DISCOVERY = 0,
    DISCOVERY_ACK = 1,
    TRANSACTION_REQ = 2,
    TRANSACTION_ACK = 3,
    REGISTER_BACKUP = 4,
    ADD_CLIENT_UPDATE = 5,
    HEARTBEAT = 6,
    NEW_PRIMARY_ANNOUNCEMENT = 7,
    STATE_UPDATE = 8,
    STATE_UPDATE_ACK = 9,
    ADD_HISTORY_ENTRY = 10,
    SERVER_DISCOVERY = 11
};

struct ClientUpdateData {
    char ip[INET_ADDRSTRLEN];
    float balance;
    int last_request;
};

struct StateUpdateData {
    Transaction transaction; // A transação completa para o histórico

    char source_ip[INET_ADDRSTRLEN];
    float source_balance;    // O *novo* saldo da origem
    
    char dest_ip[INET_ADDRSTRLEN];
    float dest_balance;      // O *novo* saldo do destino
};

struct ReqData {
    uint32_t dest_addr;
    uint32_t value;
    uint32_t seqn;
};

struct AckData {
    uint32_t seqn;
    float new_balance;
    uint32_t status;
    uint32_t last_req;
};

struct Packet {
    uint16_t type;
    union {
        ReqData req;
        AckData ack;
        ClientUpdateData client_update;
        StateUpdateData state_update;
        Transaction history_entry;
    } data;
};

std::string getCurrentFormattedTime();
bool validatePortArg(int argc, char* argv[]);
bool validatePort(int port);
int getValidatedPort(int argc, char* argv[]);

#endif // UTILS_H

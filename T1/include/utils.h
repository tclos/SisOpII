#ifndef UTILS_H
#define UTILS_H

#include <string>

#define INITIAL_BALANCE 1000.0f

enum class LogType {
    NONE,
    SUCCESS,
    DUPLICATE
};

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
    ERROR_INSUFFICIENT_FUNDS
};

struct Transaction {
    int id;
    std::string source_ip;
    std::string dest_ip;
    int value;
    int client_seqn;
};

enum PacketType {
    DISCOVERY = 0,
    DISCOVERY_ACK = 1,
    TRANSACTION_REQ = 2,
    TRANSACTION_ACK = 3
};

struct ReqData {
    uint32_t dest_addr;
    uint32_t value;
    uint32_t seqn;
};

struct AckData {
    uint32_t seqn;
    float new_balance;
};

struct Packet {
    uint16_t type;
    union {
        ReqData req;
        AckData ack;
    } data;
};

std::string getCurrentFormattedTime();
bool validatePortArg(int argc, char* argv[]);
bool validatePort(int port);
int getValidatedPort(int argc, char* argv[]);

#endif // UTILS_H

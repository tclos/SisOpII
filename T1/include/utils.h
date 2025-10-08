#ifndef UTILS_H
#define UTILS_H

#include <string>

#define INITIAL_BALANCE 1000.0f

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

#ifndef CLIENT_DTO_H
#define CLIENT_DTO_H

#include <string>
#include "utils.h"

class ClientDTO {
private:
    std::string address;
    int last_req;
    float balance;

public:
    ClientDTO(const std::string& addr)
        : address(addr), last_req(0), balance(INITIAL_BALANCE) {}

    std::string getAddress() const { return address; }
    int getLastRequest() const { return last_req; }
    float getBalance() const { return balance; }

    void setLastRequest(int req) { last_req = req; }
    void setBalance(float bal) { balance = bal; }
};

#endif // CLIENT_DTO_H
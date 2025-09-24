#include "Client.h"

Client::Client(std::string addr, int last_req, float bal)
    : address(addr), last_req(last_req), balance(bal) {}

std::string Client::getAddress() const { return address; }
int Client::getLastRequest() const { return last_req; }
float Client::getBalance() const { return balance; }

void Client::setLastRequest(int last_req) { this->last_req = last_req; }
void Client::setBalance(float bal) { balance = bal; }

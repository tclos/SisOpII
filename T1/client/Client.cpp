#include "Client.h"
#include "utils.h"
#include "clientInterface.h"

Client::Client(std::string addr, int last_req, float bal)
    : address(addr), last_req(last_req), balance(bal) {}

Client::Client(std::string addr)
    : address(addr), last_req(0), balance(0.0f) {}

std::string Client::getAddress() const { return address; }
int Client::getLastRequest() const { return last_req; }
float Client::getBalance() const { return balance; }

void Client::setLastRequest(int last_req) { this->last_req = last_req; }
void Client::setBalance(float bal) { balance = bal; }

void Client::init() {
    logInitialMessage(this->address);
}

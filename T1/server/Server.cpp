
#include "Server.h"
#include "serverInterface.h"
#include <iostream>


Server::Server(int port)
    : udp_port(port), num_transactions(0), total_transferred(0), total_balance(0) {}


int Server::getPort() const { return udp_port; }
int Server::getNumTransactions() const { return num_transactions; }
float Server::getTotalTransferred() const { return total_transferred; }
float Server::getTotalBalance() const { return total_balance; }

void Server::init(int port) {
    logInitialMessage(num_transactions, static_cast<int>(total_transferred), static_cast<int>(total_balance));
}

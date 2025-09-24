#include "Server.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

Server::Server(int port)
    : udp_port(port), num_transactions(0), total_transferred(0), total_balance(0) {}

int Server::getPort() const { return udp_port; }
int Server::getNumTransactions() const { return num_transactions; }
float Server::getTotalTransferred() const { return total_transferred; }
float Server::getTotalBalance() const { return total_balance; }

std::string Server::getCurrentFormattedTime() {
    auto now = std::time(nullptr);
    std::tm tm_struct = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm_struct, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Server::init(int port) {
    std::cout 
        << getCurrentFormattedTime() 
        << " num transactions " << num_transactions 
        << " total transferred " << static_cast<int>(total_transferred)
        << " total balance " << static_cast<int>(total_balance) 
        << std::endl;
}

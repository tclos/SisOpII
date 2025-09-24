#include "serverInterface.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string getCurrentFormattedTime() {
    auto now = std::time(nullptr);
    std::tm tm_struct = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm_struct, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void logInitialMessage(int num_transactions, int total_transferred, int total_balance) {
    std::cout 
        << getCurrentFormattedTime() 
        << " num transactions " << num_transactions 
        << " total transferred " << total_transferred
        << " total balance " << total_balance 
        << std::endl;
}

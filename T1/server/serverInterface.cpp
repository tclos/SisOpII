#include "serverInterface.h"
#include "utils.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

void logInitialMessage(int num_transactions, int total_transferred, int total_balance) {
    std::cout 
        << getCurrentFormattedTime() 
        << " num transactions " << num_transactions 
        << " total transferred " << total_transferred
        << " total balance " << total_balance 
        << std::endl;
}

void logRequisitionMessage(const std::string& client_address, int request_id, const std::string& dest_address, int value, int num_transactions, int total_transferred, int total_balance) {
    std::cout 
        << getCurrentFormattedTime() 
        << " client " << client_address
        << " id req " << request_id
        << " dest " << dest_address 
        << " value " << value
        << " num transactions " << num_transactions
        << " total transferred " << total_transferred
        << " total balance " << total_balance 
        << std::endl;
}

std::string getDuplicatedMessage(const std::string& client_address, int request_id, const std::string& dest_address, int value, int num_transactions, int total_transferred, int total_balance) {
    std::ostringstream oss;
    oss << getCurrentFormattedTime() 
        << " client " << client_address
        << " DUP!! "
        << " id req " << request_id
        << " dest " << dest_address 
        << " value " << value
        << " num transactions " << num_transactions
        << " total transferred " << total_transferred
        << " total balance " << total_balance;
    
    return oss.str();
}

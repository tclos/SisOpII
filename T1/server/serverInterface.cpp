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

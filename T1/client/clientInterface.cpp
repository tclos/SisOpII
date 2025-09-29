#include "clientInterface.h"
#include "utils.h"
#include <iostream>

void logInitialMessage(std::string address) {
    std::cout 
        << getCurrentFormattedTime() 
        << " server addr " << address
        << std::endl;
}
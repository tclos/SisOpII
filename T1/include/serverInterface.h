#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

#include <string>

std::string getCurrentFormattedTime();
void logInitialMessage(int num_transactions, int total_transferred, int total_balance);

#endif // SERVER_INTERFACE_H

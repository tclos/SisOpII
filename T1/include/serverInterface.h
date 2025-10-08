#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

#include <string>

void logInitialMessage(int num_transactions, int total_transferred, int total_balance);
void logRequisitionMessage(const std::string& client_address, int request_id, const std::string& dest_address, int value, int num_transactions, int total_transferred, int total_balance);
std::string getDuplicatedMessage(const std::string& client_address, int request_id, const std::string& dest_address, int value, int num_transactions, int total_transferred, int total_balance);

#endif // SERVER_INTERFACE_H

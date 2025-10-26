#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include "ClientDTO.h"
#include "ServerUDP.h"
#include "serverInterface.h"
#include "utils.h"

class Server {
private:
    int udp_port;
    int num_transactions;
    int total_transferred;
    int total_balance;
    ServerUDP server_socket;
    std::vector<ClientDTO> clients;
    std::vector<Transaction> transaction_history;
    ServerInterface interface;

    mutable std::mutex data_mutex;
    mutable std::condition_variable reader_cv;
    mutable std::condition_variable writer_cv;
    mutable int readers_count;
    bool writer_active;
    int writers_waiting;

    LogInfo last_log_info;
    
    bool wasClientAdded(const std::string& client_ip);
    std::vector<ClientDTO>::iterator findClient(const std::string& ip);
    TransactionStatus validateTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn);
    void executeTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn);
    void updateAndLogTransaction(const std::string& source_ip, const std::string& dest_ip, int value, int seqn);
    void setupDuplicateRequestLog(const std::string& source_ip, const std::string& dest_ip, int value, int seqn);
    std::pair<TransactionStatus, float> handleTransactionLogic(const std::string& source_ip, const std::string& dest_ip, int value, int seqn);

public:
    Server(int port);

    int getNumTransactions() const;
    int getTotalTransferred() const;
    int getTotalBalance() const;
    LogInfo getLastLogInfo() const;
    Transaction getLastTransaction() const;

    void addClient(const std::string& client_ip);
    void printClients() const;
    std::tuple<TransactionStatus, float, int> processTransaction(const std::string& source_ip, uint32_t dest_addr_int, int value, int seqn);
    void init(int port);

    void reader_lock() const;
    void reader_unlock() const;
    void writer_lock();
    void writer_unlock();
};

#endif // SERVER_H

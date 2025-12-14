#include "serverInterface.h"
#include "Server.h"
#include "utils.h"  // CORREÇÃO: Adicionada
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

ServerInterface::ServerInterface(Server& s) : server(s) {}

ServerInterface::~ServerInterface() {
    if (interface_thread.joinable()) {
        interface_thread.join();
    }
}

void ServerInterface::start() {
    logInitialMessage();
    interface_thread = std::thread(&ServerInterface::run, this);
}

void ServerInterface::notify() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        data_changed = true;
    }
    cv.notify_one();
}

void ServerInterface::run() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return data_changed; });

        server.reader_lock();

        LogInfo log_info = server.getLastLogInfo();

        if (log_info.type == LogType::SUCCESS) {
            Transaction last_transaction = server.getLastTransaction();
            logRequisitionMessage(last_transaction);
        } else if (log_info.type == LogType::DUPLICATE) {
            logDuplicatedMessage(log_info);
        }
        
        server.reader_unlock();
        
        data_changed = false;
    }
}

void ServerInterface::logInitialMessage() {
    std::cout 
        << getCurrentFormattedTime() 
        << " num transactions " << server.getNumTransactions()
        << " total transferred " << server.getTotalTransferred()
        << " total balance " << server.getTotalBalance()
        << std::endl;
}

void ServerInterface::logRequisitionMessage(const Transaction& transaction) {
    std::cout 
        << getCurrentFormattedTime() 
        << " client " << transaction.source_ip
        << " id req " << transaction.client_seqn
        << " dest " << transaction.dest_ip
        << " value " << transaction.value
        << " num transactions " << server.getNumTransactions()
        << " total transferred " << server.getTotalTransferred()
        << " total balance " << server.getTotalBalance()
        << std::endl;
}

void ServerInterface::logDuplicatedMessage(const LogInfo& log_info) {
    std::cout  << getCurrentFormattedTime() 
        << " client " << log_info.source_ip
        << " DUP'!"
        << " id req " << log_info.transaction_id
        << " dest " << log_info.dest_ip
        << " value " << log_info.value
        << " num transactions " << server.getNumTransactions()
        << " total transferred " << server.getTotalTransferred()
        << " total balance " << server.getTotalBalance()
        << std::endl;
}
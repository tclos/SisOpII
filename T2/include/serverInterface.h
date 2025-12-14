#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "utils.h"

class Server; 

class ServerInterface {
private:
    Server& server;
    std::thread interface_thread;
    
    std::mutex mtx;
    std::condition_variable cv;
    bool data_changed = false;

    void run();

    void logInitialMessage();
    void logRequisitionMessage(const Transaction& transaction);
    void logDuplicatedMessage(const LogInfo& log_info);

public:
    ServerInterface(Server& s);
    ~ServerInterface();

    void start();
    void notify();
};

#endif // SERVER_INTERFACE_H
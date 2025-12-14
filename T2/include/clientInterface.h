#ifndef CLIENT_INTERFACE_H
#define CLIENT_INTERFACE_H

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "utils.h"

class Client;

class ClientInterface {
private:
    Client& client;
    std::thread input_thread;
    std::thread communication_thread;

    std::queue<std::string> command_queue;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool finished = false;

    void userInputThread();
    void communicationThread();

    void logInitialMessage(const std::string& address);
    void logResponse(int seqn, const std::string& dest_ip, int value, const AckData& ack);
    void logTimeout(int seqn, int retry, int max_retries);
    void logError(const std::string& message);
    
    bool tryGetCommandFromQueue(std::string& out_command);
    void processCommand(const std::string& command);

public:
    ClientInterface(Client& c);

    void start();
    void shutdown();
};

#endif // CLIENT_INTERFACE_H

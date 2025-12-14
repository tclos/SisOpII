#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <utility>
#include "ClientUDP.h"
#include "utils.h"
#include <atomic>
#include <thread>
#include <mutex>

class Client {
private:
    int port;
    int sequence_number;

    std::string server_address;
    struct sockaddr_in server_sock_addr;
    ClientUDP client_socket;

    mutable std::mutex server_mutex; 

    std::thread announcement_thread;
    std::atomic<bool> running;

    void listenForAnnouncements();

public:
    Client(int port);
    ~Client();
    
    std::string discoverServer();
    std::pair<bool, AckData> executeRequestWithRetries(const std::string& dest_ip, int value);

    int getSequenceNumber() const;
    std::string getServerAddress() const;

    void incrementSequenceNumber();
};

#endif // CLIENT_H
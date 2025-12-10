#ifndef REPLICATION_MANAGER_H
#define REPLICATION_MANAGER_H

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <netinet/in.h>
#include "ServerUDP.h"
#include "utils.h"

class ReplicationManager {
private:
    ServerUDP& socket;
    
    std::vector<struct sockaddr_in> backups;
    std::mutex backups_mutex;

    struct sockaddr_in primary_addr;
    std::atomic<bool> running_monitor;
    std::atomic<bool> running_sender;
    
    std::mutex monitor_mutex;
    std::condition_variable monitor_cv;
    bool primary_alive_signal;

    std::thread monitor_thread;
    std::thread sender_thread;

    std::function<void()> onPrimaryFailure;

public:
    ReplicationManager(ServerUDP& s);
    ~ReplicationManager();

    void addBackup(const struct sockaddr_in& addr);
    void clearBackups();
    void broadcast(const Packet& p);
    void startHeartbeatSender();
    void stopHeartbeatSender();

    void setPrimaryAddress(const struct sockaddr_in& addr);
    void startHeartbeatMonitor(std::function<void()> failureCallback);
    void stopHeartbeatMonitor();
    void notifyHeartbeatReceived();
    
    // Auxiliar
    const std::vector<struct sockaddr_in>& getBackups() const;
};

#endif
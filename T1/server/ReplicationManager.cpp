#include "ReplicationManager.h"
#include <iostream>
#include <chrono>
#include <cstring>

ReplicationManager::ReplicationManager(ServerUDP& s) 
    : socket(s), 
      running_monitor(false), 
      running_sender(false), 
      primary_alive_signal(false) {}

ReplicationManager::~ReplicationManager() {
    stopHeartbeatMonitor();
    stopHeartbeatSender();
}

void ReplicationManager::addBackup(const struct sockaddr_in& addr) {
    std::lock_guard<std::mutex> lock(backups_mutex);
    backups.push_back(addr);
}

void ReplicationManager::clearBackups() {
    std::lock_guard<std::mutex> lock(backups_mutex);
    backups.clear();
}

void ReplicationManager::broadcast(const Packet& p) {
    std::lock_guard<std::mutex> lock(backups_mutex);
    for (const auto& bk : backups) {
        socket.sendPacket(p, bk);
    }
}

const std::vector<struct sockaddr_in>& ReplicationManager::getBackups() const {
    return backups;
}

void ReplicationManager::startHeartbeatSender() {
    stopHeartbeatSender();
    running_sender = true;

    sender_thread = std::thread([this]() {
        while (running_sender) {
            Packet packet{};
            packet.type = htons(HEARTBEAT);
            
            this->broadcast(packet);

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    sender_thread.detach();
}

void ReplicationManager::stopHeartbeatSender() {
    running_sender = false;
    if (sender_thread.joinable()) {
        sender_thread.join();
    }
}

void ReplicationManager::setPrimaryAddress(const struct sockaddr_in& addr) {
    primary_addr = addr;
}

void ReplicationManager::notifyHeartbeatReceived() {
    {
        std::lock_guard<std::mutex> lock(monitor_mutex);
        primary_alive_signal = true;
    }
    monitor_cv.notify_one();
}

void ReplicationManager::startHeartbeatMonitor(std::function<void()> failureCallback) {
    stopHeartbeatMonitor();
    
    onPrimaryFailure = failureCallback;
    running_monitor = true;
    
    monitor_thread = std::thread([this]() {
        while (running_monitor) {
            std::unique_lock<std::mutex> lock(monitor_mutex);
            primary_alive_signal = false;
            
            if (monitor_cv.wait_for(lock, std::chrono::seconds(3), [this]{ return primary_alive_signal; })) {
            } else {
                if (!running_monitor) break;
                
                std::cout << "[TIMEOUT] Primário não respondeu ao Heartbeat." << std::endl;
                
                lock.unlock();
                
                if (onPrimaryFailure) {
                    onPrimaryFailure();
                }
                break;
            }
        }
    });
    monitor_thread.detach();
}

void ReplicationManager::stopHeartbeatMonitor() {
    running_monitor = false;
    monitor_cv.notify_all();
    if (monitor_thread.joinable()) {
        monitor_thread.join();
    }
}
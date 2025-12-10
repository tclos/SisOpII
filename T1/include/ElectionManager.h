#ifndef ELECTION_MANAGER_H
#define ELECTION_MANAGER_H

#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include "ServerUDP.h"
#include "utils.h"

class ElectionManager {
private:
    uint32_t id;
    ServerUDP& socket;
    
    std::atomic<bool> election_in_progress;
    std::atomic<bool> election_lost;
    
    std::function<void()> onBecamePrimary;
    std::function<void(uint32_t, struct sockaddr_in)> onNewPrimaryFound;
    std::function<bool()> isCurrentRolePrimary;

public:
    ElectionManager(uint32_t id, ServerUDP& socket);
    ~ElectionManager();

    void setId(uint32_t id);

    void setCallbacks(std::function<void()> winCallback, 
                      std::function<void(uint32_t, struct sockaddr_in)> loseCallback,
                      std::function<bool()> roleCheckCallback);

    void startElection();
    
    void handleElectionPacket(const Packet& packet, const struct sockaddr_in& sender_addr);
    void handleAnswerPacket(const Packet& packet);
    void handleNewPrimaryAnnouncement(const Packet& packet, const struct sockaddr_in& sender_addr);

    bool isElectionInProgress() const;
};

#endif
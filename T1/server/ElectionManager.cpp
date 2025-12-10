#include "ElectionManager.h"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <chrono>

ElectionManager::ElectionManager(uint32_t id, ServerUDP& socket) 
    : id(id), socket(socket), election_in_progress(false), election_lost(false) {}

ElectionManager::~ElectionManager() {
}

void ElectionManager::setId(uint32_t id) {
    this->id = id;
}

void ElectionManager::setCallbacks(std::function<void()> winCallback, 
                                   std::function<void(uint32_t, struct sockaddr_in)> loseCallback,
                                   std::function<bool()> roleCheckCallback) {
    onBecamePrimary = winCallback;
    onNewPrimaryFound = loseCallback;
    isCurrentRolePrimary = roleCheckCallback;
}

bool ElectionManager::isElectionInProgress() const {
    return election_in_progress;
}

void ElectionManager::startElection() {
    std::thread([this]() {
        this->election_in_progress = true;
        this->election_lost = false;
        
        Packet packet{};
        packet.type = htons(ELECTION);
        packet.data.req.value = htonl(this->id);

        struct sockaddr_in broadcast_addr;
        memset(&broadcast_addr, 0, sizeof(broadcast_addr));
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_port = htons(socket.getPort());
        inet_pton(AF_INET, BROADCAST_ADDR, &broadcast_addr.sin_addr);
        
        socket.sendPacket(packet, broadcast_addr);
        std::cout << "[ELEIÇÃO] Iniciando eleição (ID: " << this->id << ")" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        this->election_in_progress = false;

        if (!this->election_lost) {
            std::cout << "[ELEIÇÃO] Este servidor é o novo primário." << std::endl;
            if (onBecamePrimary) onBecamePrimary();
        } else {
            std::cout << "[ELEIÇÃO] Aguardando anúncio do novo líder." << std::endl;
        }
    }).detach();
}

void ElectionManager::handleElectionPacket(const Packet& packet, const struct sockaddr_in& sender_addr) {
    uint32_t sender_id = ntohl(packet.data.req.value);

    if (this->id > sender_id) {
        Packet resp{};
        resp.type = htons(ELECTION_ANSWER);
        resp.data.req.value = htonl(this->id);
        socket.sendPacket(resp, sender_addr);

        bool am_primary = isCurrentRolePrimary ? isCurrentRolePrimary() : false;

        if (am_primary) {
            Packet ann{};
            ann.type = htons(NEW_PRIMARY_ANNOUNCEMENT);
            ann.data.req.value = htonl(this->id);
            socket.sendPacket(ann, sender_addr); 
        } 
        else if (!this->election_in_progress) {
            std::cout << "[ELEIÇÃO] Recebi de ID menor (" << sender_id << ")" << std::endl;
            startElection();
        }
    } else {
        if (this->election_in_progress) {
            this->election_lost = true;
            std::cout << "[ELEIÇÃO] Detectado ID superior (" << sender_id << ") durante eleição." << std::endl;
        }
    }
}

void ElectionManager::handleAnswerPacket(const Packet& packet) {
    uint32_t sender_id = ntohl(packet.data.req.value);
    
    if (this->election_in_progress && sender_id > this->id) {
        this->election_lost = true;
        std::cout << "[ELEIÇÃO] Recebi resposta de autoridade ID " << sender_id << ". Abortando." << std::endl;
    }
}

void ElectionManager::handleNewPrimaryAnnouncement(const Packet& packet, const struct sockaddr_in& sender_addr) {
    uint32_t new_leader_id = ntohl(packet.data.req.value);

    if (new_leader_id == this->id) {
        return;
    }
    
    bool should_surrender = false;
    bool am_primary = isCurrentRolePrimary ? isCurrentRolePrimary() : false;

    if (am_primary) {
        if (new_leader_id > this->id) {
            std::cout << "[CONFLITO] Outro Primário com ID maior (" << new_leader_id << ") detectado. Rebaixando-me." << std::endl;
            should_surrender = true;
        } else {
            std::cout << "[CONFLITO] Primário impostor com ID menor (" << new_leader_id << ") detectado. Reforçando liderança." << std::endl;
            if (onBecamePrimary) onBecamePrimary(); 
            return;
        }
    } else {
        should_surrender = true;
    }

    if (should_surrender) {
        if (this->election_in_progress) this->election_lost = true;
        
        if (onNewPrimaryFound) onNewPrimaryFound(new_leader_id, sender_addr);
    }
}

#include "Server.h"
#include "discovery.h"
#include "transactions.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>

Server::Server(int port)
    : num_transactions(0), 
    total_transferred(0), 
    total_balance(0),
    server_socket(port),
    clients(),
    transaction_history(),   
    interface(*this),
    readers_count(0),
    writer_active(false),
    writers_waiting(0),
    election_in_progress(false) 
    {
        this->server_id = getIp();

        if (this->server_id == 0) {
            std::cerr << "[ERRO] Não foi possível determinar o IP da rede. ID definido como 0." << std::endl;
        }
    }

int Server::getNumTransactions() const {
    return num_transactions;
}

int Server::getTotalTransferred() const {
    return total_transferred;
}

int Server::getTotalBalance() const {
    return total_balance;
}

LogInfo Server::getLastLogInfo() const {
    return last_log_info;
}

Transaction Server::getLastTransaction() const {
    if (transaction_history.empty()) {
        return Transaction{};
    }
    return transaction_history.back();
}

ServerRole Server::getRole() const {
    return this->current_role;
}

void Server::init(int port) {
    struct sockaddr_in client_addr;
    Packet received_packet;

    if (server_socket.createSocket() == false) {
        throw std::runtime_error("Erro ao criar o socket do servidor.");
    }

    server_socket.configureBroadcast();
    server_socket.setUpServerAddress();
    server_socket.bindSocket();

    std::string primary_ip = run_discovery_service_server(server_socket, port);

    if (!primary_ip.empty()) {
        initBackup(primary_ip, port);
    } else {
        initPrimary();
    }

    interface.start();

    while (true) {
        int n = server_socket.receivePacket(received_packet, client_addr);
        if (n <= 0) continue;

        switch (ntohs(received_packet.type)) {
            case TRANSACTION_REQ: {
                if (this->current_role == PRIMARY) {
                    Packet req_copy = received_packet;
                    handleTransactionRequest(req_copy, client_addr, *this, server_socket);
                }
                break;
            }
            case DISCOVERY: {
                handleDiscoveryPacket(server_socket, *this, client_addr);
                break;
            }
            case DISCOVERY_ACK: {
                break;
            }
            case REGISTER_BACKUP: {
                if (this->current_role == PRIMARY) {
                    registerBackup(client_addr);
                }
                break;
            }
            case ADD_CLIENT_UPDATE: {
                if (this->current_role == BACKUP) {
                    handleClientUpdate(received_packet);
                }
                break;
            }
            case STATE_UPDATE: {
                if (this->current_role == BACKUP) {
                    handleStateUpdate(received_packet);
                }
                break;
            }
            case ADD_HISTORY_ENTRY: {
                if (this->current_role == BACKUP) {
                    handleHistoryEntry(received_packet);
                }
                break;
            }
            case HEARTBEAT: {
                handleHeartbeatPacket();
                break;
            }
            case SERVER_DISCOVERY: {
                sendDiscoveryAck(client_addr);
                break;
            }
            case NEW_PRIMARY_ANNOUNCEMENT: {
                handleNewPrimaryAnnouncementPacket(received_packet, client_addr);
                break;
            }
            case ELECTION: {
                handleElectionPacket(received_packet, client_addr);
                break;
            }
            case ELECTION_ANSWER: {
                handleElectionAnswerPacket(received_packet);
                break;
            }
            default:
                std::cerr << "Tipo de pacote desconhecido recebido." << std::endl;
                break;
        }
    }

    server_socket.closeSocket();
}

void Server::printClients() const {
    reader_lock();

    std::cout << std::left << std::setw(20) << "Endereço IP"
              << std::setw(25) << "Último ID recebido"
              << std::setw(15) << "Saldo Atual" << std::endl;
    std::cout << "-----------------------------------------------------" << std::endl;

    for (const auto& client : clients) {
        std::cout << std::left << std::setw(20) << client.getAddress()
                  << std::setw(25) << client.getLastRequest()
                  << std::fixed << std::setprecision(2) << client.getBalance() << std::endl;
    }
    std::cout << "-----------------------------------------------------\n" << std::endl;
    
    reader_unlock();
}


bool Server::wasClientAdded(const std::string& client_ip) {
    std::vector<ClientDTO>::iterator it = std::find_if(clients.begin(), clients.end(), [&](const ClientDTO& c) {
        return c.getAddress() == client_ip;
    });

    if (it == clients.end()) {
        ClientDTO newClient(client_ip);
        clients.push_back(newClient);
        
        this->total_balance += newClient.getBalance();

        if (this->current_role == PRIMARY) {
            propagateClientAddition(newClient);
        }

        return true;
    }
    std::cout << "Cliente " << client_ip << " já está registado." << std::endl;
    return false;
}

std::vector<ClientDTO>::iterator Server::findClient(const std::string& ip) {
    return std::find_if(clients.begin(), clients.end(), [&](const ClientDTO& c) {
        return c.getAddress() == ip;
    });
}

void Server::addClient(const std::string& client_ip) {
    bool client_added = false;
    writer_lock();
    client_added = wasClientAdded(client_ip);
    writer_unlock();

    if (client_added) {
        interface.notify();
    }
}

TransactionStatus Server::validateTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn) {
    if (source_it == clients.end() || dest_it == clients.end()) {
        return TransactionStatus::ERROR_CLIENT_NOT_FOUND;
    }
    if (seqn <= source_it->getLastRequest()) {
        return TransactionStatus::ERROR_DUPLICATE_REQUEST;
    }
    if (seqn > source_it->getLastRequest() + 1) {
        return TransactionStatus::ERROR_OUT_OF_SEQUENCE;
    }
    if (source_it->getBalance() < value) {
        return TransactionStatus::ERROR_INSUFFICIENT_FUNDS;
    }
    return TransactionStatus::SUCCESS;
}

void Server::executeTransaction(std::vector<ClientDTO>::iterator& source_it, std::vector<ClientDTO>::iterator& dest_it, int value, int seqn) {
    source_it->setBalance(source_it->getBalance() - value);
    dest_it->setBalance(dest_it->getBalance() + value);
    source_it->setLastRequest(seqn);
}

void Server::updateAndLogTransaction(const std::string& source_ip, const std::string& dest_ip, int value, int seqn) {
    this->num_transactions++;
    this->total_transferred += value;

    Transaction new_transaction;
    new_transaction.id = this->num_transactions;

    strncpy(new_transaction.source_ip, source_ip.c_str(), INET_ADDRSTRLEN);
    new_transaction.source_ip[INET_ADDRSTRLEN - 1] = '\0';

    strncpy(new_transaction.dest_ip, dest_ip.c_str(), INET_ADDRSTRLEN);
    new_transaction.dest_ip[INET_ADDRSTRLEN - 1] = '\0';

    new_transaction.value = value;
    new_transaction.client_seqn = seqn;
    this->transaction_history.push_back(new_transaction);
}

void Server::setupDuplicateRequestLog(const std::string& source_ip, const std::string& dest_ip, int value, int seqn) {
    last_log_info.type = LogType::DUPLICATE;
    last_log_info.transaction_id = seqn;
    last_log_info.value = value;
    last_log_info.source_ip = source_ip;
    last_log_info.dest_ip = dest_ip;
}

std::pair<TransactionStatus, float> Server::handleTransactionLogic(const std::string& source_ip, const std::string& dest_ip, int value, int seqn) {
    auto source_it = findClient(source_ip);
    auto dest_it = findClient(dest_ip);

    TransactionStatus status = validateTransaction(source_it, dest_it, value, seqn);

    if (status == TransactionStatus::SUCCESS) {
        executeTransaction(source_it, dest_it, value, seqn);
        updateAndLogTransaction(source_ip, dest_ip, value, seqn);
        last_log_info.type = LogType::SUCCESS;
        return {TransactionStatus::SUCCESS, source_it->getBalance()};
    } 
    
    if (status == TransactionStatus::ERROR_DUPLICATE_REQUEST || status == TransactionStatus::ERROR_OUT_OF_SEQUENCE) {
        setupDuplicateRequestLog(source_ip, dest_ip, value, seqn);
        return {status, source_it->getBalance()};
    }

    float balance = (source_it == clients.end()) ? -1.0f : source_it->getBalance();
    return {status, balance};
}

std::tuple<TransactionStatus, float, int> Server::processTransaction(const std::string& source_ip, uint32_t dest_addr_int, int value, int seqn) {
    writer_lock();
    
    struct in_addr dest_ip_struct;
    dest_ip_struct.s_addr = dest_addr_int;
    char dest_ip_cstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &dest_ip_struct, dest_ip_cstr, INET_ADDRSTRLEN);
    std::string dest_ip(dest_ip_cstr);
    
    std::pair<TransactionStatus, float> transaction_result = handleTransactionLogic(source_ip, dest_ip, value, seqn);
    TransactionStatus status = transaction_result.first;
    float balance = transaction_result.second;

    if (status == TransactionStatus::SUCCESS && this->current_role == PRIMARY) {
        Transaction new_tx = this->transaction_history.back();

        auto source_it = findClient(source_ip);
        auto dest_it = findClient(dest_ip);
        
        propagateTransaction(new_tx, *source_it, *dest_it);
    }
    
    bool should_notify = (status == TransactionStatus::SUCCESS || 
                          status == TransactionStatus::ERROR_DUPLICATE_REQUEST || 
                          status == TransactionStatus::ERROR_OUT_OF_SEQUENCE);

    auto source_it = findClient(source_ip);
    int last_req = (source_it == clients.end()) ? 0 : source_it->getLastRequest();

    writer_unlock();

    if (should_notify) {
        interface.notify();
    }
    
    return {status, balance, last_req};
}

void Server::reader_lock() const {
    std::unique_lock<std::mutex> lock(data_mutex);
    reader_cv.wait(lock, [this] { return !writer_active && writers_waiting == 0; });
    readers_count++;
}

void Server::reader_unlock() const {
    std::lock_guard<std::mutex> lock(data_mutex);
    readers_count--;
    if (readers_count == 0) {
        writer_cv.notify_one();
    }
}

void Server::writer_lock() {
    std::unique_lock<std::mutex> lock(data_mutex);
    writers_waiting++;
    writer_cv.wait(lock, [this] { return readers_count == 0 && !writer_active; });
    writers_waiting--;
    writer_active = true;
}

void Server::writer_unlock() {
    std::lock_guard<std::mutex> lock(data_mutex);
    writer_active = false;
    if (writers_waiting > 0) {
        writer_cv.notify_one();
    } else {
        reader_cv.notify_all();
    }
}

void Server::propagateClientAddition(const ClientDTO& new_client) {
    Packet packet;
    packet.type = htons(ADD_CLIENT_UPDATE);

    strncpy(packet.data.client_update.ip, 
            new_client.getAddress().c_str(), 
            INET_ADDRSTRLEN);
            
    packet.data.client_update.ip[INET_ADDRSTRLEN - 1] = '\0';
    
    packet.data.client_update.balance = new_client.getBalance();
    packet.data.client_update.last_request = new_client.getLastRequest();

    for (const auto& backup_addr : backup_servers) {
        server_socket.sendPacket(packet, backup_addr);
    }
}

void Server::propagateTransaction(const Transaction& new_tx, 
                                  const ClientDTO& source_client, 
                                  const ClientDTO& dest_client) 
{
    Packet packet;
    packet.type = htons(STATE_UPDATE);

    packet.data.state_update.transaction = new_tx;

    strncpy(packet.data.state_update.source_ip, 
            source_client.getAddress().c_str(), 
            INET_ADDRSTRLEN);
    packet.data.state_update.source_ip[INET_ADDRSTRLEN - 1] = '\0';
    packet.data.state_update.source_balance = source_client.getBalance();

    strncpy(packet.data.state_update.dest_ip, 
            dest_client.getAddress().c_str(), 
            INET_ADDRSTRLEN);
    packet.data.state_update.dest_ip[INET_ADDRSTRLEN - 1] = '\0';
    packet.data.state_update.dest_balance = dest_client.getBalance();

    for (const auto& backup_addr : backup_servers) {
        server_socket.sendPacket(packet, backup_addr);
    }
}

void Server::handleStateUpdate(const Packet& packet) {
    const StateUpdateData& data = packet.data.state_update;
    
    Transaction tx = data.transaction;
    std::string source_ip = data.source_ip;
    float source_balance = data.source_balance;
    std::string dest_ip = data.dest_ip;
    float dest_balance = data.dest_balance;

    writer_lock();

    auto source_it = findClient(source_ip);
    if (source_it == clients.end()) {
        clients.emplace_back(source_ip);
        source_it = findClient(source_ip);
    }

    auto dest_it = findClient(dest_ip);
    if (dest_it == clients.end()) {
        clients.emplace_back(dest_ip);
        dest_it = findClient(dest_ip);
    }

    source_it->setBalance(source_balance);
    dest_it->setBalance(dest_balance);
    
    source_it->setLastRequest(tx.client_seqn);

    this->transaction_history.push_back(tx);
    this->num_transactions++;
    this->total_transferred += tx.value;

    last_log_info.type = LogType::SUCCESS;
    last_log_info.transaction_id = tx.id;
    last_log_info.value = tx.value;
    last_log_info.source_ip = source_ip;
    last_log_info.dest_ip = dest_ip;

    writer_unlock();

    interface.notify();
}

void Server::handleClientUpdate(const Packet& packet) {
    
    std::string ip(packet.data.client_update.ip);
    float balance = packet.data.client_update.balance;
    int last_request = packet.data.client_update.last_request;
    
    writer_lock();
    
    auto it = findClient(ip);
    if (it == clients.end()) {
        ClientDTO new_client(ip);
        
        new_client.setBalance(balance); 
        new_client.setLastRequest(last_request);
        
        clients.push_back(new_client);
        
        this->total_balance += balance; 
        
        writer_unlock();
        interface.notify();
    } else {
        writer_unlock();
    }
}

void Server::handleHistoryEntry(const Packet& packet) {
    writer_lock();
    this->transaction_history.push_back(packet.data.history_entry);
    this->num_transactions++; //
    this->total_transferred += packet.data.history_entry.value; //
    
    writer_unlock();
    
    interface.notify();
}

void Server::registerBackup(const struct sockaddr_in& backup_addr) {
    
    std::cout << "[REPLICAÇÃO] Novo backup. Iniciando sincronização" << std::endl;

    std::vector<ClientDTO> client_snapshot;
    std::vector<Transaction> history_snapshot;

    writer_lock();
    client_snapshot = this->clients;
    history_snapshot = this->transaction_history;
    
    this->backup_servers.push_back(backup_addr);
    writer_unlock();

    for (const auto& client : client_snapshot) {
        Packet packet{};
        packet.type = htons(ADD_CLIENT_UPDATE);

        strncpy(packet.data.client_update.ip, 
                client.getAddress().c_str(), 
                INET_ADDRSTRLEN);
        packet.data.client_update.ip[INET_ADDRSTRLEN - 1] = '\0';
        packet.data.client_update.balance = client.getBalance();
        packet.data.client_update.last_request = client.getLastRequest();

        server_socket.sendPacket(packet, backup_addr);
    }

    for (const auto& tx : history_snapshot) {
        Packet packet{};
        packet.type = htons(ADD_HISTORY_ENTRY);
        
        packet.data.history_entry = tx; 

        server_socket.sendPacket(packet, backup_addr);
    }

    std::cout << "[REPLICAÇÃO] Sincronização do backup concluída." << std::endl;
}

void Server::startHeartbeatSender() {
    std::thread([this]() {
        while (this->current_role == PRIMARY) {
            Packet packet{};
            packet.type = htons(HEARTBEAT);

            reader_lock();
            for (const auto& backup_addr : this->backup_servers) {
                server_socket.sendPacket(packet, backup_addr);
            }
            reader_unlock();

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

void Server::startHeartbeatMonitor() {
std::thread([this]() {
        while (this->current_role == BACKUP) {
            std::unique_lock<std::mutex> lock(primary_mutex);
            primary_alive = false;

            if (primary_cv.wait_for(lock, std::chrono::seconds(3), 
                                    [this]{ return primary_alive; })) {
            } else {
                std::cout << "Timeout do Primário! Iniciando Eleição" << std::endl;
                
                lock.unlock();
                
                startElection();
                break;
            }
        }
    }).detach();
}

void Server::promoteToPrimary() {
    writer_lock();
    if (this->current_role == BACKUP) {
        this->current_role = PRIMARY;
        
        this->backup_servers.clear();
        
        writer_unlock();

        startHeartbeatSender();

        announceNewPrimary();
    } else {
        writer_unlock();
    }
}

void Server::announceNewPrimary() {
    Packet packet{};
    packet.type = htons(NEW_PRIMARY_ANNOUNCEMENT);
    packet.data.req.value = htonl(this->server_id);

    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(server_socket.getPort());
    inet_pton(AF_INET, BROADCAST_ADDR, &broadcast_addr.sin_addr);

    server_socket.sendPacket(packet, broadcast_addr);

    std::cout << "[REPLICAÇÃO] Anúncio de novo líder enviado via Broadcast" << std::endl;
}

void Server::startElection() {
    this->election_in_progress = true; 
    this->election_lost = false;
    
    Packet packet{};
    packet.type = htons(ELECTION);
    packet.data.req.value = htonl(this->server_id);

    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(server_socket.getPort());
    inet_pton(AF_INET, BROADCAST_ADDR, &broadcast_addr.sin_addr);
    
    server_socket.sendPacket(packet, broadcast_addr);
    std::cout << "[ELEIÇÃO] Iniciando eleição " << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    this->election_in_progress = false;

    if (!this->election_lost) {
        std::cout << "[ELEIÇÃO] Sou o novo primário." << std::endl;
        promoteToPrimary();
    } else {        
        startHeartbeatMonitor(); 
    }
}

void Server::handleElectionMsg(const Packet& packet, const struct sockaddr_in& sender_addr) {
    uint32_t sender_id = ntohl(packet.data.req.value);
    std::cout << "[ELEIÇÃO] Recebido ID: " << sender_id << " (Meu ID: " << this->server_id << ")" << std::endl;

    if (this->server_id > sender_id) {
        Packet resp{};
        resp.type = htons(ELECTION_ANSWER);
        resp.data.req.value = htonl(this->server_id);
        server_socket.sendPacket(resp, sender_addr);

        if (!this->election_in_progress) {
            this->election_in_progress = true; 
            
            std::thread([this](){ startElection(); }).detach();
        }
    }
}

uint32_t Server::getIp() {
    struct ifaddrs *interfaces = nullptr;
    struct ifaddrs *temp_addr = nullptr;
    uint32_t found_id = 0;
    
    if (getifaddrs(&interfaces) == 0) {
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr && temp_addr->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in* s_addr = (struct sockaddr_in*) temp_addr->ifa_addr;
                uint32_t ip_val = ntohl(s_addr->sin_addr.s_addr);
                
                if ((ip_val >> 24) != 127) { 
                    found_id = ip_val;
                    
                    char ipStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(s_addr->sin_addr), ipStr, INET_ADDRSTRLEN);
                    std::cout << "[Server ID definido via IP: " << ipStr << "]" << std::endl;
                    
                    break;
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    freeifaddrs(interfaces);
    return found_id;
}

void Server::initBackup(const std::string& primary_ip, int port) {
    this->current_role = BACKUP;
    std::cout << "Primário encontrado em " << primary_ip << ". Iniciando como BACKUP." << std::endl;

    memset(&primary_server_addr, 0, sizeof(primary_server_addr));
    primary_server_addr.sin_family = AF_INET;
    primary_server_addr.sin_port = htons(port);
    inet_pton(AF_INET, primary_ip.c_str(), &primary_server_addr.sin_addr);

    Packet reg_packet;
    reg_packet.type = htons(REGISTER_BACKUP);
    server_socket.sendPacket(reg_packet, primary_server_addr);

    startHeartbeatMonitor();
}

void Server::initPrimary() {
    std::cout << "Nenhum primário encontrado. Iniciando eleição." << std::endl;
    this->current_role = BACKUP;
    
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        startElection();
    }).detach();
}

void Server::handleHeartbeatPacket() {
    if (this->current_role == BACKUP) {
        std::lock_guard<std::mutex> lock(primary_mutex);
        this->primary_alive = true;
        primary_cv.notify_one();
    }
}

void Server::sendDiscoveryAck(const struct sockaddr_in& client_addr) {
    if (this->current_role == PRIMARY) {
        Packet response;
        response.type = htons(DISCOVERY_ACK);
        server_socket.sendPacket(response, client_addr);
    }
}

void Server::handleNewPrimaryAnnouncementPacket(const Packet& packet, const struct sockaddr_in& sender_addr) {
    uint32_t new_leader_id = ntohl(packet.data.req.value);
    std::cout << "[REPLICAÇÃO] Novo primário anunciado: ID " << new_leader_id << std::endl;

    if (this->election_in_progress) {
        this->election_lost = true;
    }

    if (this->current_role == BACKUP) {
        std::lock_guard<std::mutex> lock(primary_mutex);
        this->primary_alive = true;
        primary_cv.notify_one();
    }
    
    if (new_leader_id != this->server_id) {
        this->current_role = BACKUP;
        this->backup_servers.clear();
        
        memset(&primary_server_addr, 0, sizeof(primary_server_addr));
        primary_server_addr = sender_addr;
        
        Packet reg;
        reg.type = htons(REGISTER_BACKUP);
        server_socket.sendPacket(reg, primary_server_addr);
    }
}

void Server::handleElectionPacket(const Packet& packet, const struct sockaddr_in& sender_addr) {
    uint32_t sender_id = ntohl(packet.data.req.value);
    
    if (sender_id > this->server_id) {
        if (this->election_in_progress) {
            this->election_lost = true;
            std::cout << "[ELEIÇÃO] Detectado ID superior (" << sender_id << ")" << std::endl;
        }
    } else {
        Packet resp{};
        resp.type = htons(ELECTION_ANSWER);
        resp.data.req.value = htonl(this->server_id);
        server_socket.sendPacket(resp, sender_addr);
    }
}

void Server::handleElectionAnswerPacket(const Packet& packet) {
    uint32_t sender_id = ntohl(packet.data.req.value);
    if (this->election_in_progress && sender_id > this->server_id) {
        this->election_lost = true;
        std::cout << "[ELEIÇÃO] Recebi ordem de parada de ID " << sender_id << "." << std::endl;
    }
}
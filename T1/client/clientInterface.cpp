#include "clientInterface.h"
#include "Client.h"
#include "transactions.h"
#include <iostream>

ClientInterface::ClientInterface(Client& c) : client(c) {}

void ClientInterface::start() {
    try {
        std::string server_addr = client.discoverServer();
        logInitialMessage(server_addr);

        input_thread = std::thread(&ClientInterface::userInputThread, this);
        communication_thread = std::thread(&ClientInterface::communicationThread, this);

        input_thread.join();
        communication_thread.join();

    } catch (const std::runtime_error& e) {
        logError(e.what());
    }
}

void ClientInterface::shutdown() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        finished = true;
    }
    condition.notify_all();
}

void ClientInterface::userInputThread() {
    for (std::string line; std::getline(std::cin, line);) {
        if (line.empty()) continue;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            command_queue.push(line);
        }
        condition.notify_one();
    }
    shutdown(); // CTRL+D pressionado
}

void ClientInterface::communicationThread() {
    std::string command;
    while (tryGetCommandFromQueue(command)) {
        processCommand(command);
    }
}

bool ClientInterface::tryGetCommandFromQueue(std::string& out_command) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    condition.wait(lock, [this] { return !command_queue.empty() || finished; });

    if (finished && command_queue.empty()) {
        return false;
    }

    out_command = command_queue.front();
    command_queue.pop();
    return true;
}

void ClientInterface::processCommand(const std::string& command) {
    std::string dest_ip;
    int value;
    if (!validateInput(command, dest_ip, value)) {
        return;
    }

    auto result = client.executeRequestWithRetries(dest_ip, value);

    if (result.first) {
        AckData ack_data = result.second;
        TransactionStatus status = static_cast<TransactionStatus>(ntohl(ack_data.status));

        switch (status) {
            case TransactionStatus::SUCCESS:
                logResponse(client.getSequenceNumber() - 1, dest_ip, value, ack_data);
                client.incrementSequenceNumber();
                break;
            case TransactionStatus::ERROR_INSUFFICIENT_FUNDS:
                logError("Saldo insuficiente para realizar a transação.");
                break;
            case TransactionStatus::ERROR_CLIENT_NOT_FOUND:
                logError("Cliente de origem ou destino não encontrado.");
                break;
            case TransactionStatus::ERROR_DUPLICATE_REQUEST:
                logError("Requisição duplicada ou fora de ordem detectada pelo servidor.");
                logResponse(client.getSequenceNumber() - 1, dest_ip, value, ack_data);
                client.incrementSequenceNumber();
                break;
            default:
                logError("Recebida resposta com status desconhecido do servidor.");
                break;
        }
    } else {
        logError("Não foi possível obter resposta do servidor para a requisição #" + std::to_string(client.getSequenceNumber() - 1));
    }
}

void ClientInterface::logInitialMessage(const std::string& address) {
    std::cout << getCurrentFormattedTime() << " server addr " << address << std::endl;
}

void ClientInterface::logResponse(int seqn, const std::string& dest_ip, int value, const AckData& ack) {
    std::cout << getCurrentFormattedTime()
              << " server " << client.getServerAddress()
              << " id_req " << seqn
              << " dest " << dest_ip
              << " value " << value
              << " new_balance " << ack.new_balance
              << std::endl;
}

void ClientInterface::logTimeout(int seqn, int retry, int max_retries) {
    std::cout << "Timeout! Reenviando requisição #" << seqn
              << " (" << retry << "/" << max_retries << ")" << std::endl;
}

void ClientInterface::logError(const std::string& message) {
    std::cerr << "Erro: " << message << std::endl;
}
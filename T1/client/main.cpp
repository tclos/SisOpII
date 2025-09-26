#include <iostream>
#include "Client.h"
#include "utils.h"
#include <string>

int main(int argc, char* argv[]) {
    int port = getValidatedPort(argc, argv);
    if (port == -1) return 1;

    std::string dummyAddress = "10.1.1.20";

    Client client(dummyAddress);
    client.init();

    return 0;
}
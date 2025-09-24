#ifndef CLIENT_H
#define CLIENT_H

#include <string>

class Client {
    private:
        std::string address;
        int last_req;
        float balance;
    
    public:
        Client(std::string addr, int last_req, float bal);

        std::string getAddress() const;
        int getLastRequest() const;
        float getBalance() const;

        void setLastRequest(int last_req);
        void setBalance(float bal);
};

#endif // CLIENT_H

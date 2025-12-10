CXX = g++
CXXFLAGS = -std=c++14 -Wall -Iinclude -pthread

OBJDIR = obj
SRCS = $(wildcard server/*.cpp client/*.cpp discovery/*.cpp udp/*.cpp utils.cpp)
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

TARGETS = servidor cliente

SERVER_OBJS = \
    $(OBJDIR)/server/main.o \
    $(OBJDIR)/server/Server.o \
    $(OBJDIR)/server/serverInterface.o \
    $(OBJDIR)/discovery/discoveryServer.o \
    $(OBJDIR)/udp/ServerUDP.o \
    $(OBJDIR)/utils.o \
    $(OBJDIR)/transactions/transactionServer.o \
    $(OBJDIR)/server/ElectionManager.o \
    $(OBJDIR)/server/ReplicationManager.o

CLIENT_OBJS = \
    $(OBJDIR)/client/main.o \
    $(OBJDIR)/client/Client.o \
    $(OBJDIR)/client/clientInterface.o \
    $(OBJDIR)/discovery/discoveryClient.o \
    $(OBJDIR)/udp/ClientUDP.o \
    $(OBJDIR)/utils.o \
    $(OBJDIR)/transactions/transactionClient.o

all: $(TARGETS)

servidor: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_OBJS)

cliente: $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_OBJS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) servidor cliente

.PHONY: all clean

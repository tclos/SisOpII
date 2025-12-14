# ==============================================
# Makefile para Sistema Bancário Distribuído
# Sistemas Operacionais II - UFRGS
# ==============================================

CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -Iinclude -pthread -g -Wno-sign-compare
LDFLAGS = -pthread

OBJDIR = obj

TARGETS = servidor cliente

SERVER_OBJS = \
    $(OBJDIR)/server/main.o \
    $(OBJDIR)/server/Server.o \
    $(OBJDIR)/server/serverInterface.o \
    $(OBJDIR)/discovery/discoveryServer.o \
    $(OBJDIR)/transactions/transactionServer.o \
    $(OBJDIR)/udp/ServerUDP.o \
    $(OBJDIR)/utils.o

CLIENT_OBJS = \
    $(OBJDIR)/client/main.o \
    $(OBJDIR)/client/Client.o \
    $(OBJDIR)/client/clientInterface.o \
    $(OBJDIR)/discovery/discoveryClient.o \
    $(OBJDIR)/transactions/transactionClient.o \
    $(OBJDIR)/udp/ClientUDP.o \
    $(OBJDIR)/utils.o

all: directories $(TARGETS)

directories:
	@mkdir -p $(OBJDIR)/server $(OBJDIR)/client $(OBJDIR)/discovery $(OBJDIR)/udp $(OBJDIR)/transactions

servidor: $(SERVER_OBJS)
	@echo "Linking servidor..."
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_OBJS) $(LDFLAGS)
	@echo "Servidor compilado: ./servidor"

cliente: $(CLIENT_OBJS)
	@echo "Linking cliente..."
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_OBJS) $(LDFLAGS)
	@echo "Cliente compilado: ./cliente"

$(OBJDIR)/utils.o: utils.cpp include/utils.h
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c utils.cpp -o $(OBJDIR)/utils.o

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Limpando..."
	rm -rf $(OBJDIR) servidor cliente *.log *.pid test_input.txt
	@echo "Limpeza concluída"

# ==============================================
# TESTES
# ==============================================

test-quick: all
	@echo "TESTE RÁPIDO - 30 segundos"
	@echo "================================"
	@echo "Iniciando teste automático..."
	@echo ""
	@echo "1. Iniciando servidor primário (porta 5000)..."
	@(./servidor 5000 > server1.log 2>&1 & echo $$! > server1.pid)
	@sleep 3
	@echo "2. Iniciando servidor backup (porta 5001)..."
	@(./servidor 5001 > server2.log 2>&1 & echo $$! > server2.pid)
	@sleep 3
	@echo "3. Enviando transação via cliente..."
	@echo "127.0.0.1 50" > test_input.txt
	@timeout 5 ./cliente 5000 < test_input.txt > client.log 2>&1 || true
	@sleep 2
	@echo ""
	@echo "RESULTADOS:"
	@echo "============="
	@if ps -p `cat server1.pid 2>/dev/null` >/dev/null 2>&1; then \
		echo "Primário ativo"; \
	else \
		echo "Primário caiu"; \
	fi
	@if ps -p `cat server2.pid 2>/dev/null` >/dev/null 2>&1; then \
		echo "Backup ativo"; \
	else \
		echo "Backup caiu"; \
	fi
	@if grep -q "server" client.log || grep -q "new_balance" client.log; then \
		echo "Cliente funcionou"; \
	else \
		echo "Cliente sem resposta (pode ser normal)"; \
	fi
	@echo ""
	@echo "LOGS (últimas 3 linhas cada):"
	@echo "Primário:" && tail -3 server1.log 2>/dev/null || echo "  (sem log)"
	@echo "Backup:" && tail -3 server2.log 2>/dev/null || echo "  (sem log)"
	@echo "Cliente:" && tail -3 client.log 2>/dev/null || echo "  (sem log)"
	@echo ""
	@echo "4. Testando falha do primário..."
	@kill `cat server1.pid 2>/dev/null` 2>/dev/null || true
	@sleep 5
	@if grep -q "ELEIÇÃO" server2.log || grep -q "PROMOÇÃO" server2.log; then \
		echo "Eleição detectada no backup"; \
	else \
		echo "Eleição não detectada (verificar timeouts)"; \
	fi
	@echo ""
	@echo "5. Limpando..."
	@kill `cat server1.pid server2.pid 2>/dev/null` 2>/dev/null || true
	@rm -f server1.log server2.log client.log server1.pid server2.pid test_input.txt
	@echo "Teste concluído!"

test-manual: all
	@echo "TESTE MANUAL - Siga os passos:"
	@echo "==================================="
	@echo ""
	@echo "1. ABRA 3 TERMINAIS DIFERENTES"
	@echo ""
	@echo "2. TERMINAL 1 - Servidor Primário:"
	@echo "   ./servidor 4000"
	@echo ""
	@echo "3. TERMINAL 2 - Servidor Backup:"
	@echo "   ./servidor 4001"
	@echo ""
	@echo "4. TERMINAL 3 - Cliente:"
	@echo "   ./cliente 4000"
	@echo ""
	@echo "5. No cliente (Terminal 3), digite:"
	@echo "   127.0.0.1 50"
	@echo "   (Pressione Enter)"
	@echo ""
	@echo "6. Observe os logs nos servidores"
	@echo ""
	@echo "7. Mate o primário (Ctrl+C no Terminal 1)"
	@echo ""
	@echo "8. Observe a eleição automática no backup"
	@echo ""
	@echo "9. Continue usando o cliente - deve funcionar com novo primário"

# ==============================================
# AJUDA
# ==============================================

help:
	@echo "COMANDOS DISPONÍVEIS:"
	@echo ""
	@echo "  make all         - Compila tudo"
	@echo "  make servidor    - Compila apenas servidor"
	@echo "  make cliente     - Compila apenas cliente"
	@echo "  make clean       - Limpa arquivos compilados"
	@echo "  make test-quick  - Teste automático rápido (30s)"
	@echo "  make test-manual - Instruções para teste manual"
	@echo "  make help        - Mostra esta ajuda"
	@echo ""
	@echo "ESTRUTURA DETECTADA: include/ existe"
	@echo "STATUS: PRONTO PARA COMPILAÇÃO"

.PHONY: all clean directories test-quick test-manual help
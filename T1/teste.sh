#!/bin/bash

# --- Configurações do Teste de Estresse ---
# Quantidade de clientes que serão iniciados em paralelo.
NUM_CLIENTS=5
# Quantidade de transações que cada cliente tentará realizar.
NUM_TRANSACTIONS_PER_CLIENT=10
# Nome da sua imagem Docker.
DOCKER_IMAGE="discovery-app"
# Nome da sua rede Docker.
DOCKER_NETWORK="discovery-net"
# Prefixo dos IPs que serão atribuídos aos clientes.
IP_PREFIX="172.18.0."
# Sufixo inicial para os IPs dos clientes (para não conflitar com o servidor).
STARTING_IP_SUFFIX=20
# Porta em que os clientes e o servidor estão operando.
PORT=4000
# Valor máximo para uma única transação.
MAX_TRANSACTION_VALUE=50
# Nome do diretório para salvar os logs dos clientes.
LOG_DIR="stress_test_logs"

# --- Preparação do Ambiente ---
CLIENT_IPS=()
CLIENT_CONTAINER_NAMES=()

echo "--- Preparando o ambiente para o teste de estresse ---"

# Limpa e recria o diretório de logs para garantir uma execução limpa.
rm -rf ${LOG_DIR}
mkdir -p ${LOG_DIR}
echo "Diretório de logs '${LOG_DIR}' preparado."

# Gera a lista de IPs e nomes de containers para os 15 clientes.
for i in $(seq 0 $((NUM_CLIENTS - 1))); do
    ip_suffix=$((STARTING_IP_SUFFIX + i))
    ip="${IP_PREFIX}${ip_suffix}"
    CLIENT_IPS+=("$ip")
    CLIENT_CONTAINER_NAMES+=("client-stress-${ip_suffix}")
done

# Função de limpeza para parar e remover containers caso o script seja interrompido (Ctrl+C).
cleanup() {
    echo -e "\n--- Interrupção recebida, limpando containers... ---"
    # O comando 'docker ps -q ...' lista os IDs dos containers que correspondem ao nome.
    running_containers=$(docker ps -q -f name="^/client-stress-")
    if [ ! -z "$running_containers" ]; then
        echo "Parando containers de teste restantes..."
        docker stop $running_containers > /dev/null
    fi
    echo "Limpeza concluída."
    exit 1
}

# Associa a função de limpeza aos sinais de interrupção e término.
trap cleanup SIGINT SIGTERM

# --- FASE 1: Descoberta e Registro ---
echo "--- Fase 1: Descoberta e Registro de todos os clientes ---"
echo "Iniciando cada cliente com uma transação trivial para garantir o registro no servidor..."

for i in $(seq 0 $((NUM_CLIENTS - 1))); do
    SOURCE_IP="${CLIENT_IPS[$i]}"
    CONTAINER_NAME="discovery-${CLIENT_CONTAINER_NAMES[$i]}" # Nome temporário para a fase de descoberta

    # O comando é uma consulta do próprio saldo, o que força a descoberta e o registro.
    DISCOVERY_COMMAND="${SOURCE_IP} 0"

    # Inicia um container para cada cliente, executa o comando e sai.
    # A saída é descartada pois esta fase é apenas para registro.
    (
      echo -e "${DISCOVERY_COMMAND}" | docker run --rm -i --network ${DOCKER_NETWORK} --ip ${SOURCE_IP} --name ${CONTAINER_NAME} ${DOCKER_IMAGE} ./cliente ${PORT} > /dev/null 2>&1
    ) &
done

wait # Espera todos os processos de descoberta em background terminarem.
echo "--- Fase 1 Concluída: Todos os clientes foram registrados no servidor. ---"
sleep 2 # Pequeno intervalo para garantir a consistência do servidor.
echo "" # Linha em branco para melhor legibilidade.


# --- FASE 2: Execução das Transações Concorrentes ---
echo "--- Fase 2: Iniciando ${NUM_CLIENTS} clientes em paralelo para as transações... ---"
echo "Cada cliente fará ${NUM_TRANSACTIONS_PER_CLIENT} transações."
echo "Os logs de cada cliente serão salvos em '${LOG_DIR}/'."

# Inicia todos os clientes em processos de background para as transações.
for i in $(seq 0 $((NUM_CLIENTS - 1))); do
    SOURCE_IP="${CLIENT_IPS[$i]}"
    CONTAINER_NAME="${CLIENT_CONTAINER_NAMES[$i]}"
    COMMANDS=""

    # Gera uma lista de comandos de transação aleatórios para este cliente.
    for j in $(seq 1 $NUM_TRANSACTIONS_PER_CLIENT); do
        # Escolhe um cliente de destino aleatório que não seja ele mesmo.
        DEST_INDEX=$(( RANDOM % NUM_CLIENTS ))
        while [ $DEST_INDEX -eq $i ]; do
            DEST_INDEX=$(( RANDOM % NUM_CLIENTS ))
        done
        DEST_IP="${CLIENT_IPS[$DEST_INDEX]}"

        # Gera um valor aleatório para a transação (de 1 a MAX_TRANSACTION_VALUE).
        VALUE=$(( (RANDOM % MAX_TRANSACTION_VALUE) + 1 ))

        # Adiciona o comando formatado à lista de comandos.
        COMMANDS+="${DEST_IP} ${VALUE}\n"
        # Adiciona um pequeno delay aleatório (entre 0.1 e 0.5 segundos) para simular um usuário.
        COMMANDS+="sleep 0.$((RANDOM % 5 + 1))\n"
    done

    echo "Iniciando transações para o cliente ${CONTAINER_NAME} (${SOURCE_IP})"
    
    # Inicia o container do cliente em background, salvando o log na pasta designada.
    (
      echo -e "${COMMANDS}" | docker run --rm -i --network ${DOCKER_NETWORK} --ip ${SOURCE_IP} --name ${CONTAINER_NAME} ${DOCKER_IMAGE} ./cliente ${PORT} > "${LOG_DIR}/logs_cliente_${SOURCE_IP}.txt" 2>&1
    ) &

done

# --- Finalização ---
echo -e "\n--- Todos os clientes iniciaram as suas transações. Aguardando a conclusão... ---"
# O comando 'wait' aguarda todos os processos em background (os containers de transação) terminarem.
wait

echo -e "\n--- Teste de estresse concluído. ---"
echo "Verifique os logs do servidor e os arquivos na pasta '${LOG_DIR}/'."
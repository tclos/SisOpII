# Estágio 1: Compilação (Builder)
# Usamos uma imagem com o compilador GCC para criar os executáveis
FROM gcc:11 AS builder

# Define o diretório de trabalho dentro do contêiner
WORKDIR /app

# Copia todos os ficheiros do projeto para o contêiner
COPY . .

# Executa o Makefile para compilar tudo de forma limpa
RUN make clean && make

# Estágio 2: Imagem Final
# Usamos uma imagem base leve (debian-slim) que não tem o compilador
FROM debian:stable-slim

# Define o diretório de trabalho
WORKDIR /app

# Copia APENAS os executáveis compilados do estágio de build
COPY --from=builder /app/servidor .
COPY --from=builder /app/cliente .

# Expõe a porta 4000 (informativo, pois --network host a torna acessível)
EXPOSE 4000/udp

# Define o comando padrão para executar o servidor quando o contêiner iniciar
CMD ["./servidor", "4000"]
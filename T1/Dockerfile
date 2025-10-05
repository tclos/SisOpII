FROM gcc:11 AS builder

WORKDIR /app

COPY . .

RUN make clean && make

FROM debian:stable-slim

WORKDIR /app

COPY --from=builder /app/servidor .
COPY --from=builder /app/cliente .

EXPOSE 4000/udp

CMD ["./servidor", "4000"]

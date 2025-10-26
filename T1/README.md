## Como compilar

```
make clean
```

```
make
```

## Como testar durante o desenvolvimento

Buildar o container

```
docker build --platform linux/amd64 -t discovery-app .
```

Criar uma sub-rede no container

```
docker network create --subnet=172.18.0.0/16 discovery-net
```


### Inicializar servidor

```
docker run --rm --network discovery-net --name servidor-app discovery-app ./servidor 4000
```

### Inicializar cliente

```
docker run --rm -it --network discovery-net --ip 172.18.0.21 discovery-app ./cliente 4000
```

É possível testar com IPs diferentes:

```
docker run --rm -it --network discovery-net --ip 172.18.0.22 discovery-app ./cliente 4000
```

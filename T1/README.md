## Como testar

Buildar o container

```
docker build --platform linux/amd64 -t discovery-app .
```

### Inicializar servidor

```
docker run --rm --network host --platform linux/amd64 discovery-app ./servidor 4000
```

### Inicializar cliente

```
docker run --rm --network host --platform linux/amd64 discovery-app ./cliente 4000
```

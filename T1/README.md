## Server

How to compile

```
g++ -Iinclude -o servidor server/main.cpp server/Server.cpp server/serverInterface.cpp client/Client.cpp utils.cpp
```

How to init

```
./servidor 4000
```

## Client

How to compile

```
g++ -Iinclude -o cliente client/main.cpp client/Client.cpp client/clientInterface.cpp utils.cpp
```

How to init

```
./cliente 4000
```
all: servidor cliente

servidor: server_main.o discovery.o
	g++ -std=c++11 -Wall -o servidor server_main.o discovery.o

cliente: client_main.o discovery.o
	g++ -std=c++11 -Wall -o cliente client_main.o discovery.o

server_main.o: server_main.cpp discovery.h
	g++ -std=c++11 -Wall -c server_main.cpp

client_main.o: client_main.cpp discovery.h
	g++ -std=c++11 -Wall -c client_main.cpp

discovery.o: discovery.cpp discovery.h
	g++ -std=c++11 -Wall -c discovery.cpp

clean:
	rm -f *.o servidor cliente
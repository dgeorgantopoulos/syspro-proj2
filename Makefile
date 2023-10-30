all: client server

client: client.o functions.o work.o
	gcc -g3 client.o functions.o work.o -o remoteClient -lpthread

server: server.o functions.o work.o
	gcc -g3 server.o functions.o work.o -o dataServer -lpthread

client.o: client.c
	gcc -g3 -c client.c -lpthread

server.o: server.c
	gcc -g3 -c server.c -lpthread

work.o : work.c work.h
	gcc -g3 -c work.c -lpthread

functions.o : functions.c
	gcc -g3 -c functions.c -lpthread

clean:
	rm main main.o work.o functions.o client dataServer remoteClient server.o client.o
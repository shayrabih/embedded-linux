HDRS = PROJECT.h EVB2.h

all: evb2 client server

evb2 : evb2.o
	arm-linux-gcc evb2.o -lpthread -o evb2

evb2.o : evb2.c $(HDRS)
	arm-linux-gcc -lpthread -c evb2.c

client : PC_client.o
	gcc PC_client.o -o client

PC_client.o : PC_client.c PROJECT.h
	gcc -c PC_client.c

server : PC_server.o
	gcc PC_server.o -o server

PC_server.o: PC_server.c PROJECT.h
	gcc -c PC_server.c


PHONY : clean

clean :
	rm *.o 


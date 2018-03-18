CC = g++
CFLAG = -c

ALL.O = util.o server.o client.o

all: $(ALL.O)
	$(CC) util.o server.o -o s
	$(CC) util.o client.o -o c

util.o: util.cpp util.h
	$(CC) $(CFLAG) util.cpp

server.o: server.cpp util.h util.o
	$(CC) $(CFLAG) server.cpp

client.o: client.cpp util.h util.o
	$(CC) $(CFLAG) client.cpp
clean:
	rm *.o *.out

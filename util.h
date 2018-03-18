#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#define BACKLOG 10
#define CHUNK_SIZE 1024
#define TRUE 1
#define FALSE 0
#define INIT "init"

vector<string> normalize_args(int argc, const char *argv[]);
vector<string> tokenize_words(string args, string delimiter);
string produceBuffer(int n, ...);
int myread(char *buffer, int len);
void *get_in_addr(struct sockaddr *sa);
void setHints(struct addrinfo *hints, int ai_flags);
int runServer(struct addrinfo **p, struct addrinfo *serveinfo);
int runClient(struct addrinfo **p, struct addrinfo *servinfo);
int sendAll(int fd, char *buf, int len);

#endif

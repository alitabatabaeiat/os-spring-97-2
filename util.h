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
#include <bits/stdc++.h>
#include <dirent.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

#define BACKLOG 10
#define CHUNK_SIZE 1024
#define TRUE 1
#define FALSE 0
#define IDLE 0
#define BUSY 1
#define PIPE_NAME "./named_pipe"
#define LOGS_FILE_NAME "./logs.txt"
#define INIT "init"
#define QUIT "quit"

vector<string> normalize_args(int argc, const char *argv[]);
vector<string> tokenize_words(string args, string delimiter);
string produce_buffer(int n, ...);
string itoa(int number);
string dtoa(double number);
void logs_process();
void write_in_named_pipe(string str);
string find_min(vector<vector<string> > &str);
string concat_list(vector<string> l);
vector<string> read_pipes(int pipefds[][2], int size);
string normalize_candidate(string candidate, int start_to, int end_from);
string lowercase(string in);
void find_word(string candidate, vector<string> p1, vector<string> p2, vector<string> p3, double &sum);
double get_file_score(string dir, vector<string> p1, vector<string> p2, vector<string> p3);
vector<string> open_dir(string path);
string solve(string base_dir, vector<string> p1, vector<string> p2, vector<string> p3);
void *get_in_addr(struct sockaddr *sa);
void set_hints(struct addrinfo *hints, int ai_flags);
int run_server(struct addrinfo **p, struct addrinfo *serveinfo);
int run_client(struct addrinfo **p, struct addrinfo *servinfo);
int send_all(int fd, char *buf, int len);

#endif

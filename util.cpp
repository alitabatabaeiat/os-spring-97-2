#include "util.h"

vector<string> normalize_args(int argc, const char *argv[]) {
  vector<string> res;
  for (int i = 1; i < argc; i++) {
    string temp(argv[i]);
    temp = temp.substr(3, (temp.length() - 4));
    res.push_back(temp);
  }
  return res;
}

vector<string> tokenize_words(string str, string delimiter) {
  vector<string> res;
  int pos;
  while ((pos = str.find(delimiter)) != -1) {
    string token = str.substr(0, pos);
    str.erase(0, pos + delimiter.length());
    res.push_back(token);
  }
  res.push_back(str);
  return res;
}

string produceBuffer(int n, ...) {
  vector<string> s;
  va_list vl;
  va_start(vl, n);
  for (int i = 0; i < n; i++)
    s.push_back(va_arg(vl, string));
  va_end(vl);
  
  string buffer = s[0];
  for (int i = 1; i < s.size(); i++) {
    buffer += ";";
    buffer += s[i];
  }
  return buffer;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in*)sa)->sin_addr);

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void setHints(struct addrinfo *hints, int ai_flags) {
  hints->ai_family = AF_UNSPEC;
  hints->ai_socktype = SOCK_STREAM;
  if (ai_flags != 0)
    hints->ai_flags = AI_PASSIVE;
}

int runServer(struct addrinfo **p, struct addrinfo *servinfo) {
  int socketfd;
  int yes = 1;
  for((*p) = servinfo; (*p) != NULL; (*p) = (*p)->ai_next) {
    if ((socketfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) {
      cout << "ERR! socket" << endl;
      continue;
    }
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      cout << "ERR! setsockopt" << endl;
      return 0;
    }
    if (bind(socketfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) {
      close(socketfd);
      cout << "ERR! bind" << endl;
      continue;
    }
    break;
  }

  return socketfd;
}

int runClient(struct addrinfo **p, struct addrinfo *servinfo) {
  int socketfd;
  for((*p) = servinfo; (*p) != NULL; (*p) = (*p)->ai_next) {
    if ((socketfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) {
      cout << "ERR! socket" << endl;
      continue;
    }
    if (connect(socketfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) {
      close(socketfd);
      cout << "ERR! connect" << endl;
      continue;
    }
    break;
  }

  return socketfd;
}

int sendAll(int fd, char *buf, int len) {
  int total = 0;
  int bytesleft = len;
  int n;
  while(total < len) {
    n = send(fd, buf + total, bytesleft, 0);
    if (n == -1)
      break;
    total += n;
    bytesleft -= n;
  }
  return n == -1 ? FALSE : total;
}

#include "util.h"


int main(int argc, char const *argv[]) {
  if (argc != 7) {
    cout << "ERR! argc must be equl to 7!" << endl;
    return 0;
  }

  vector<string> args = normalize_args(argc, argv);
  const char *ip = args[0].c_str();
  const char *port = args[1].c_str();
  int serverfd;
  struct addrinfo hints, *servinfo, *p;
  char serverRes[CHUNK_SIZE];
  memset(serverRes, '\0', CHUNK_SIZE);

  string username = args[2];

  // client connect to server
  memset(&hints, 0, sizeof hints);
  setHints(&hints, FALSE);

  if (getaddrinfo(ip, port, &hints, &servinfo) != 0) {
    cout << "ERR! getaddrinfo" << endl;
    return 0;
  }

  serverfd = runClient(&p, servinfo);
  freeaddrinfo(servinfo);
  if (p == NULL) {
    cout << "ERR! failed to connect " << endl;
    return 0;
  }
  cout << "connecting to server on port " << port << endl;

  string buffer = produceBuffer(5, (string)INIT, username, args[3], args[4], args[5]);
  sendAll(serverfd, (char *)buffer.c_str(), buffer.length());
  int nbytes = recv(serverfd, serverRes, CHUNK_SIZE, 0);
  cout << "Server: " << serverRes << endl;
  close(serverfd);
  return 0;
}

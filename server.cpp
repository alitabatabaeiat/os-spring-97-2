#include "util.h"

int  main(int argc, char const *argv[]) {
  if (argc != 5) {
    cout << "ERR! ERR! argc must be equl to 5" << endl;
    return 0;
  }

  vector<string> args = normalize_args(argc, argv);
  const char *ip = args[0].c_str();
  const char *port = args[1].c_str();
  const char *db = args[2].c_str();
  int num_of_workers = atoi(args[3].c_str());
  cerr << num_of_workers << endl;
  int socketfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage remoteaddr;
  socklen_t sin_size;
  char remoteIP[INET6_ADDRSTRLEN];
  fd_set master;
  fd_set read_fds;
  int fdmax;
  char buffer[CHUNK_SIZE] = {0};

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  memset(&hints, 0, sizeof hints);
  setHints(&hints, TRUE);
  if (getaddrinfo(NULL, args[1].c_str(), &hints, &servinfo) != 0) {
    cout << "ERR! getaddrinfo" << endl;
    return 0;
  }

  socketfd = runServer(&p, servinfo);
  freeaddrinfo(servinfo);

  if (p == NULL)  {
    cout << "ERR! failed to bind" << endl;
    return 0;
  }

  if (listen(socketfd, BACKLOG) == -1) {
    cout << "ERR! listen" << endl;
    return 0;
  }

  FD_SET(socketfd, &master);
  fdmax = socketfd;

  FD_SET(STDIN_FILENO, &master);
  if (STDIN_FILENO > fdmax)
    fdmax = STDIN_FILENO;
  cout << "waiting for connections..." << endl;

  while (1) {
    read_fds = master;
    if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
      cout << "ERR! select" << endl;
      return 0;
    }
    for(int i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_fds)) {
        if (i == socketfd) {
          socklen_t addrlen = sizeof remoteaddr;
          new_fd = accept(socketfd, (struct sockaddr *)&remoteaddr, &addrlen);
          if (new_fd == -1) {
            cout << "ERR! accept" << endl;
          } else {
            FD_SET(new_fd, &master);
            if (new_fd > fdmax)
              fdmax = new_fd;
            cout << "new connection from "
                 << (char *)inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*) &remoteaddr), remoteIP, INET6_ADDRSTRLEN)
                 << " on socket "
                 << new_fd << endl;
          }
        } else if (i == STDIN_FILENO) {
          // myread(buffer, CHUNK_SIZE);
          // removeSpaces(buffer);
        } else {
          int nbytes;
          if ((nbytes = recv(i, buffer, CHUNK_SIZE, 0)) <= 0) {
            if (nbytes == 0) {
              cout << "socket " << i << " hung up" << endl;
            }
            else
              cout << "ERR! recv" << endl;
            close(i);
            FD_CLR(i, &master);
          } else {
            // cout << "Client " << i << ": " << buffer << endl;
            // sendAll(i, buffer, CHUNK_SIZE);
          }
        } // END handle data from client
      } // END got new incoming connection
    } // END looping through file descriptors
  }

  return 0;
}

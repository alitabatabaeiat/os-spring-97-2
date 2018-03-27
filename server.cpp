#include "util.h"

int  main(int argc, char const *argv[]) {
  if (argc != 5) {
    cout << "ERR! ERR! argc must be equl to 5" << endl;
    return 0;
  }

  vector<string> args = normalize_args(argc, argv);
  const char *ip = args[0].c_str();
  const char *port = args[1].c_str();
  string db(args[2].c_str());
  int num_of_workers = atoi(args[3].c_str());
  int socketfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage remoteaddr;
  socklen_t sin_size;
  char remoteIP[INET6_ADDRSTRLEN];
  fd_set master;
  fd_set read_fds;
  int fdmax;
  char buffer[CHUNK_SIZE] = {0};
  vector<int> workers_pid;
  vector<int> workers_state(num_of_workers, IDLE);
  int pipefds[num_of_workers][2];

  if (mkfifo(PIPE_NAME, 0666) != 0 && errno != EEXIST)
    cout << "ERR! mkfifo (" << strerror(errno) << ")" << endl;

  int log_pid = fork();
  if (log_pid > 0) {
    for (int i = 0; i < num_of_workers; i++) {
      int res = pipe(pipefds[i]);
      if (res != 0) cout << "workers pipe error" << endl;
      workers_pid.push_back(fork());
      if (workers_pid.back() == 0) break;
      else if (workers_pid.back() < 0) cout << "workers fork error" << endl;
    }
  }

  if (log_pid > 0 && workers_pid.back() > 0) { // parent proceess

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    memset(&hints, 0, sizeof hints);
    set_hints(&hints, TRUE);
    if (getaddrinfo(NULL, args[1].c_str(), &hints, &servinfo) != 0) {
      cout << "ERR! getaddrinfo" << endl;
      return 0;
    }

    socketfd = run_server(&p, servinfo);
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

    bool quit = false;
    while (!quit) {
      read_fds = master;
      if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
        cout << "ERR! select" << endl;
        return 0;
      }
      for(int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &read_fds)) {
          memset(buffer, '\0', CHUNK_SIZE);
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
            string input;
            cin >> input;
            if (input == QUIT)
              quit = true;
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
              string message = buffer;
              vector<string> tokens = tokenize_words(message, ";");
              if (tokens[0] == INIT) {
                string username = tokens[1];
                vector<string> p1 = tokenize_words(tokens[2], ",");
                vector<string> p2 = tokenize_words(tokens[3], ",");
                vector<string> p3 = tokenize_words(tokens[4], ",");
                string info = username + ": " + itoa(p1.size() + p2.size() + p3.size());
                write_in_named_pipe(info);
                bool worker_found = false;
                for (int j = 0; j < workers_pid.size(); j++) {
                  if (workers_state[j] == BUSY) {
                    continue;
                  }
                  worker_found = true;
                  workers_state[j] = BUSY;
                  string buf = itoa(i) + ";" + tokens[2] + ";" + tokens[3] + ";" + tokens[4];
                  write(pipefds[j][1], buf.c_str(), buf.length());
                  break;
                }
                if (!worker_found) {
                  cout << "worker not found!" << endl;
                }
              }
            }
          } // END handle data from client
        } // END got new incoming connection
      } // END looping through file descriptors
    }
    if (unlink(PIPE_NAME) != 0)
      cout << "ERR! unlink (" << strerror(errno) << ")" << endl;
  } else if (log_pid == 0) { // logs process
    logs_process();
  } else { // workers process
    while (1) {
      char buf[CHUNK_SIZE] = {0};
      read(pipefds[workers_pid.size() - 1][0], buf, CHUNK_SIZE);
      vector<string> tokens = tokenize_words(buf, ";");
      int clientfd = atoi(tokens[0].c_str());
      vector<string> p1 = tokenize_words(tokens[1], ",");
      vector<string> p2 = tokenize_words(tokens[2], ",");
      vector<string> p3 = tokenize_words(tokens[3], ",");
      string res = solve(db, p1, p2, p3);
      vector<string> scores = tokenize_words(res, ";");
      if (unlink("scores.txt") != 0 && errno != ENOENT)
        cout << "ERR! unlink (" << strerror(errno) << ")" << endl;
      ofstream file("scores.txt", ios::app);
      for (int i = 0; i < scores.size(); i++)
        file << scores[i] << endl;
      file.close();
      send_all(clientfd, (char*)res.c_str(), res.size());
    }

  }

  return 0;
}

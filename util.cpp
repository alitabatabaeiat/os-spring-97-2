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

string produce_buffer(int n, ...) {
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

string itoa(int n) {
    int i, sign;
    string res;

    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        res += n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        res += '-';
    reverse(res.begin(), res.end());
    return res;
}

void logs_process() {
  vector<string> batch;
  if (unlink(LOGS_FILE_NAME) != 0 && errno != ENOENT)
    cout << "ERR! unlink (" << strerror(errno) << ")" << endl;
  while (1) {
    int fifofd = open(PIPE_NAME, O_RDONLY);
    if (fifofd == -1)
      cout << "ERR! open (" << strerror(errno) << ")" << endl;
    else {
      char data[CHUNK_SIZE] = {0};
      read(fifofd, data, CHUNK_SIZE);
      batch.push_back(data);
      close(fifofd);
      cout << batch.size() << endl;
      if (batch.size() == 5) {
        ofstream logs_file(LOGS_FILE_NAME, ios::app);
        if (logs_file.is_open()) {
          for (int i = 0; i < batch.size(); i++)
            logs_file << batch[i] << endl;
          batch.clear();
        }
      }
    }
  }
}

void write_in_named_pipe(string str) {
  int fifofd = open(PIPE_NAME, O_WRONLY);
  if (fifofd == -1)
    cout << "ERR! open (" << strerror(errno) << ")" << endl;
  else {
    write(fifofd, str.c_str(), str.length());
    close(fifofd);
  }
}

bool open_dir(string path, vector<string> &files) {
  struct dirent *pDirent;
  DIR *pDir = opendir((path + "/").c_str());

  if(pDir == NULL)
    return false;
  while ((pDirent = readdir(pDir)) != NULL)
    if(strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0)
      files.push_back(pDirent->d_name);
  closedir (pDir);
  return true;
}

string get_file_score(string dir, vector<string> p1, vector<string> p2, vector<string> p3) {
    int sum = 0;
    ifstream file(dir.c_str());
    if(file.is_open()) {
      string candidate;
      while(file >> candidate)
  			find_word(candidate, p1, p2, p3, sum);
        file.close();
        cout << "sum in file " << dir << ": " << sum << endl;
    }
    return itoa(sum);
}

string solve(string base_dir, vector<string> p1, vector<string> p2, vector<string> p3) {
    string sum;
    vector<string> files;
    bool is_dir = open_dir(base_dir, files);
    // cout << base_dir << ": " << is_dir << endl;
    int pipefds[files.size()][2];
    int piperet[files.size()];

    for(int i = 0; i < files.size() || !is_dir; i++) {
      piperet[i] = pipe(pipefds[i]);
      if(fork() == 0) {
        // cout << base_dir << ": " << is_dir << endl;

          // calculate sum here and send it to parent

        if(is_dir) {
          // cout << base_dir + "/" + files[i] << endl;
          sum = solve(base_dir + "/" + files[i], p1, p2, p3);
          // cout << "acc sum : " << sum << endl;
          piperet[i] = write(pipefds[i][1], sum.c_str(), sum.length());
        } else { // its a file
          cout << base_dir << endl;
          sum = get_file_score(base_dir, p1, p2, p3);
          cout << base_dir << " -> " << sum << endl;
          piperet[i] = write(pipefds[i][1], sum.c_str(), sum.length());
        }

        if(piperet[i] != sum.length()){
          cout << getpid() << ": ";
          cout << "pipe did not send the expected value!" << endl;

          cout << getpid() << ": ";
          cout << sum << " " << piperet[i] << endl;
          exit(2);
        } else if(piperet[i] == -1){
          cout << "ERROR writing" << endl;
        }
        exit(0);
        // send sum to father
      }
    }

    char buffer[CHUNK_SIZE];
    memset(buffer, '\0', CHUNK_SIZE);
    for(int i = 0 ; i < files.size() ; i++) {

        piperet[i] = read(pipefds[i][0], buffer, CHUNK_SIZE); /* Read from pipe */

        if(piperet[i] == -1){
            cout << "ERROR reading pipe" << endl;
        }
        if (piperet[i] != strlen(buffer)) {
            cout << "Read did not return expected value" << endl;
            exit(4);
        }
        string tmpsum = buffer;
        memset(buffer, '\0', CHUNK_SIZE);
        sum = add(sum, buffer);
    }
    return sum;
}

string add(string a, string b) {
    return itoa(atoi(a.c_str()) + atoi(b.c_str()));
}

string lowercase(string in) {
	for (int i = 0 ; i < in.size(); i++)
  	if (in[i] <= 'Z' && in[i] >= 'A')
    	in[i] = in[i] - ('Z'-'z');
  return in;
}

string normalize_candidate(string candidate, int start_to, int end_from) {
	string res;
	for (int i = 0; i < candidate.size(); i++) {
		if (i == start_to) {
			i = end_from - 1;
			res += candidate.substr(start_to, end_from - start_to);
			continue;
		}
		if (candidate[i] >= 'a' && candidate[i] <= 'z')
			res += candidate[i];
	}
	return res;
}

void find_word(string candidate, vector<string> p1, vector<string> p2, vector<string> p3, int &sum) {
	candidate = lowercase(candidate);
  int state = 1;
  for (int i = 0; i < (p1.size() + p2.size() + p3.size()); i++) {
    if (i == p1.size()) state = 2;
    else if (i == p2.size()) state = 3;
    string word;
    if (state == 1) word = p1[i];
    else if (state == 2) word = p2[i - p1.size()];
    else if (state == 3) word = p3[i - p2.size()];
    int x = candidate.find(word);
    if (x == -1)
      continue;
    if (normalize_candidate(candidate, x, x + word.size()) == word) {
      if (state == 1) sum += 3;
      else if (state == 2) sum += 1.5;
      else if (state == 3) sum += 1;
    }
  }
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in*)sa)->sin_addr);

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void set_hints(struct addrinfo *hints, int ai_flags) {
  hints->ai_family = AF_UNSPEC;
  hints->ai_socktype = SOCK_STREAM;
  if (ai_flags != 0)
    hints->ai_flags = AI_PASSIVE;
}

int run_server(struct addrinfo **p, struct addrinfo *servinfo) {
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

int run_client(struct addrinfo **p, struct addrinfo *servinfo) {
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

int send_all(int fd, char *buf, int len) {
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

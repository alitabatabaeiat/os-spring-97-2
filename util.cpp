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
  if (str.empty())
    return res;
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

string itoa(int number) {
  ostringstream oss;
  oss << number;
  return oss.str();
}

string dtoa(double number) {
  ostringstream oss;
  oss << number;
  return oss.str();
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

string find_min(vector<vector<string> > &str) {
  double min = 1000;
  int index = -1;
  for (int i = 0; i < str.size(); i++) {
    if (str[i].size() == 0) continue;
    string s = str[i][0];
    double score = atof(s.substr(s.find_last_of(" ") + 1).c_str());
    if (score < min) {
      min = score;
      index = i;
    }
  }
  if (index != -1) {
    string s = str[index][0];
    str[index].erase(str[index].begin());
    return s;
  }
  return "--- MIN NOT FOUND!!!";
}

string concat_list(vector<string> l) {
  vector<vector<string> > tokenized;
  string res = "";
  int counter = 0;
  for (int i = 0; i < l.size(); i++) {
    tokenized.push_back(tokenize_words(l[i], ";"));
    counter += tokenized.back().size();
  }
  for (int i = 0; i < counter; i++) {
    string min = find_min(tokenized);
    if (i == 0)
      res = min;
    else
      res += ";" + min;
  }
  return res;
}

vector<string> read_pipes(int pipefds[][2], int size) {
  vector<string> res(size);
  for (int i = 0; i < size; i++) {
    char buf[CHUNK_SIZE] = {0};
    read(pipefds[i][0], buf, CHUNK_SIZE);
    string str = buf;
    res[i] = str;
  }
  return res;
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

string lowercase(string in) {
	for (int i = 0 ; i < in.size(); i++)
  	if (in[i] <= 'Z' && in[i] >= 'A')
    	in[i] = in[i] - ('Z'-'z');
  return in;
}

void find_word(string candidate, vector<string> p1, vector<string> p2, vector<string> p3, double &sum) {
	candidate = lowercase(candidate);
  int state = 1;
  for (int i = 0; i < (p1.size() + p2.size() + p3.size()); i++) {
    if (i == p1.size()) state = 2;
    if (i == p1.size() + p2.size()) state = 3;
    string word;
    if (state == 1) word = p1[i];
    else if (state == 2) word = p2[i - p1.size()];
    else if (state == 3) word = p3[i - (p1.size() + p2.size())];
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

double get_file_score(string dir, vector<string> p1, vector<string> p2, vector<string> p3) {
    double sum = 0;
    ifstream file(dir.c_str());
    if(file.is_open()) {
      string candidate;
      while(file >> candidate)
  			find_word(candidate, p1, p2, p3, sum);
      file.close();
    }
    return sum;
}

vector<string> open_dir(string path) {
  vector<string> files;
  struct dirent *pDirent;
  DIR *pDir = opendir((path + "/").c_str());

  if(pDir == NULL)
    return files;
  while ((pDirent = readdir(pDir)) != NULL)
    if(strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0 && strcmp(pDirent->d_name, ".DS_Store") != 0)
      files.push_back(pDirent->d_name);
  closedir (pDir);
  return files;
}

string solve(string base_dir, vector<string> p1, vector<string> p2, vector<string> p3) {
    double sum;
    vector<string> files = open_dir(base_dir);
    int pipefds[files.size()][2];

    for(int i = 0; i < files.size(); i++) {
      if (pipe(pipefds[i]) == -1)
        cout << "ERR! pipe (" << strerror(errno) << ")" << endl;
      if(fork() == 0) {
        string buf;
        if(files[i].find(".txt") == -1) {
          buf = solve(base_dir + "/" + files[i], p1, p2, p3);
        } else { // its a file
          sum = get_file_score(base_dir + "/" + files[i], p1, p2, p3);
          buf = base_dir + "/" + files[i] + " " + dtoa(sum);
        }
        write(pipefds[i][1], buf.c_str(), buf.length());
        exit(EXIT_SUCCESS);
      }
    }

    vector<string> scores = read_pipes(pipefds, files.size());
    return concat_list(scores);
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

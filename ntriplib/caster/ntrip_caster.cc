#include "caster/ntrip_caster.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

#include "util/ntrip_util.h"


static int EpollRegister(const int &epoll_fd, const int &fd) {
  struct epoll_event ev;
  int ret, flags;

  // Important: make the fd non-blocking.
  flags = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  ev.events = EPOLLIN;
  // ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = fd;
  do {
      ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
  } while (ret < 0 && errno == EINTR);

  return ret;
}

static int EpollDeregister(const int &epoll_fd, const int &fd) {
  int ret;

  do {
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
  } while (ret < 0 && errno == EINTR);

  return ret;
}

static void PrepareBufferLine(const char *buf, const int &buf_len,
                              std::vector<std::string> &buffer_line) {
  char *temp = new char[buf_len + 1];
  memset(temp, 0x0, buf_len + 1);
  memcpy(temp, buf, buf_len + 1);
  char *result = strtok(temp, "\n");
  while (result != nullptr) {
    std::string line(result);
    buffer_line.push_back(line);
    result = strtok(NULL, "\n");
  }
  reverse(buffer_line.begin(), buffer_line.end());
  delete [] temp;
}

static void ClearClientConnectInMountPoint(std::list<int> &list) {
  if (!list.empty()) {
    auto it = list.begin();
    while (it != list.end()) {
      close(*it);
      it = list.erase(it);
    }
  }
}
static void ClearAllConnectInMountPoint(std::list<MountPoint> &list) {
  if (!list.empty()) {
    auto it = list.begin();
    while (it != list.end()) {
      ClearClientConnectInMountPoint(it->client_socket_list);
      close(it->server_fd);
      it = list.erase(it);
    }
  }
}

NtripCaster::~NtripCaster() {
  if (listen_sock_ > 0) {
    close(listen_sock_);
  }
  if (epoll_fd_ > 0) {
    close(epoll_fd_);
  }
  if (epoll_events_ != nullptr) {
    delete [] epoll_events_;
  }
  ClearAllConnectInMountPoint(mount_point_list_);
  ntrip_str_list_.clear();
}

bool NtripCaster::Init(const int &port, const int &sock_count) {
  max_count_ = sock_count;
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  listen_sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock_ == -1) {
    exit(1);
  }

  if (bind(listen_sock_, reinterpret_cast<struct sockaddr*>(&server_addr),
           sizeof(struct sockaddr)) == -1) {
    exit(1);
  }

  if (listen(listen_sock_, 5) == -1) {
    exit(1);
  }

  epoll_events_ = new struct epoll_event[sock_count];
  if (epoll_events_ == nullptr) {
    exit(1);
  }
  epoll_fd_ = epoll_create(sock_count);
  EpollRegister(epoll_fd_, listen_sock_);
  return true;
}

bool NtripCaster::Init(const char *ip, const int &port, const int &sock_count) {
  max_count_ = sock_count;
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip);

  listen_sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock_ == -1) {
    exit(1);
  }

  if (bind(listen_sock_, reinterpret_cast<struct sockaddr*>(&server_addr),
           sizeof(struct sockaddr)) == -1) {
    exit(1);
  }

  if (listen(listen_sock_, 5) == -1) {
    exit(1);
  }

  epoll_events_ = new struct epoll_event[sock_count];
  if (epoll_events_ == nullptr) {
    exit(1);
  }
  epoll_fd_ = epoll_create(sock_count);
  EpollRegister(epoll_fd_, listen_sock_);
  return true;
}

int NtripCaster::AcceptNewClient() {
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(struct sockaddr_in));
  socklen_t clilen = sizeof(struct sockaddr);
  int new_sock = accept(listen_sock_, (struct sockaddr*)&client_addr, &clilen);
  EpollRegister(epoll_fd_, new_sock);
  return new_sock;
}

int NtripCaster::RecvData(const int &sock, char *recv_buf) {
  char buf[1024] = {0};
  int len = 0;
  int ret = 0;

  while (ret >= 0) {
    ret = recv(sock, buf, sizeof(buf), 0);
    if (ret <= 0) {
      EpollDeregister(epoll_fd_, sock);
      close(sock);
      break;
    } else if (ret < 1024) {
      memcpy(recv_buf, buf, ret);
      len += ret;
      break;
    } else {
      memcpy(recv_buf, buf, sizeof(buf));
      len += ret;
    }
  }

  return ret <= 0 ? ret : len;
}

int NtripCaster::SendData(const int &sock,
                          const char *send_buf, const int &buf_len) {
  int len = 0;
  int ret = 0;

  while (len < buf_len) {
    if (buf_len < 1024) {
      ret = send(sock, send_buf + len, buf_len, 0);
    } else {
      ret = send(sock, send_buf + len, 1024, 0);
    }
    if (ret <= 0) {
      EpollDeregister(epoll_fd_, sock);
      close(sock);
      break;
    } else {
      len += ret;
    }

    if (ret < 1024) {
      break;
    }
  }

  return ret <= 0 ? ret : len;
}

int NtripCaster::NtripCasterWait(const int &time_out) {
  return epoll_wait(epoll_fd_, epoll_events_, max_count_, time_out);
}

void NtripCaster::Run(const int &time_out) {
  int ret;
  int alive_count;
  char *recv_buf = new char[MAX_LEN];
  char *send_buf = new char[MAX_LEN];

  while (1) {
    ret = NtripCasterWait(time_out);
    if (ret == 0) {
      printf("Time out\n");
      continue;
    } else if (ret == -1) {
      printf("Error\n");
    } else {
      alive_count = ret;
      for (int i = 0; i < alive_count; ++i) {
        if (epoll_events_[i].data.fd == listen_sock_) {
          // If the server listens to the EPOLLIN events,
          // accept new client and add this socket to epoll listen list.
          if (epoll_events_[i].events & EPOLLIN) {
            AcceptNewClient();
          }
        } else {
          if (epoll_events_[i].events & EPOLLIN) {
            memset(recv_buf, 0, MAX_LEN);
            int ret = RecvData(epoll_events_[i].data.fd, recv_buf);
            // Received count is zero, socket error or disconnect,
            // we need remove this socket form epoll listen list.
            if (ret > 0) {
              // Start parsing received's remote data.
              if (ParseData(epoll_events_[i].data.fd, recv_buf, ret) < 0) {
                close(epoll_events_[i].data.fd);
              }
            } else if (ret == 0) {
              DealDisconnect(epoll_events_[i].data.fd);
            }
          } else if (epoll_events_[i].events & EPOLLOUT) {
            SendData(epoll_events_[i].data.fd, send_buf, strlen(send_buf));
            memset(send_buf, 0, MAX_LEN);
          }
        }
      }
    }
  }
  delete [] send_buf;
  delete [] recv_buf;
}

void NtripCaster::DealDisconnect(const int &sock) {
  if (!mount_point_list_.empty()) {
    auto it = mount_point_list_.begin();
    while (it != mount_point_list_.end()) {
      if (it->server_fd == sock) {  // It is ntrip server.
        printf("ntrip server disconnect\n");
        ClearClientConnectInMountPoint(it->client_socket_list);
        // Remove mount point information from source table list.
        std::string mount_point_name(it->mount_point_name);
        std::string ntrip_str("STR;" + mount_point_name + ";");
        auto str_it = ntrip_str_list_.begin();
        while (str_it != ntrip_str_list_.end()) {
          if (str_it->find(ntrip_str) != std::string::npos) {
            ntrip_str_list_.erase(str_it);
            break;
          }
          ++str_it;
        }
        mount_point_list_.erase(it);
        break;
      } else {  // is ntrip client.
        bool find_sock = false;
        auto cit = it->client_socket_list.begin();
        while (cit != it->client_socket_list.end()) {
          if (*cit == sock) {
            printf("ntrip client disconnect\n");
            it->client_socket_list.erase(cit);
            find_sock = true;
            break;
          }
          ++cit;
        }
        if (find_sock == true) {
          break;
        }
      }
      ++it;
    }
  }
  close(sock);
}

int NtripCaster::ParseData(const int &sock,
                           char* recv_buf, const int &buf_len) {
  int retval = -1;
  std::vector<std::string> buffer_line;

  // printf("Received [%d] btyes data:\n", buf_len);
  // PrintChar(recv_buf, buf_len);
  std::string str = recv_buf;
  if ((str.find("GET /") != std::string::npos) ||
      (str.find("POST /") != std::string::npos)) {
    PrepareBufferLine(recv_buf, buf_len, buffer_line);
    str = buffer_line.back();
    // Server request to connect to Caster.
    if ((str.find("POST /") != std::string::npos) &&
        (str.find("HTTP/1.1") != std::string::npos)) {
      PrintChar(recv_buf, buf_len);
      retval = DealServerConnectRequest(buffer_line, sock);
    } else if ((str.find("GET /") != std::string::npos) &&
               (str.find("HTTP/1.1") != std::string::npos ||
                str.find("HTTP/1.0") != std::string::npos)) {
      PrintChar(recv_buf, buf_len);
      retval = DealClientConnectRequest(buffer_line, sock);
    }
  } else {
    // Data sent by Server, it needs to be forwarded to connected client.
    if ((retval = TryToForwardServerData(sock, recv_buf, buf_len)) < 0) {
      // If Caster as a base station, it maybe needs to deal the GGA data
      // sent by the ntrip client, now it's just printing.
      if (str.find("$GPGGA,") != std::string::npos ||
          str.find("$GNGGA,") != std::string::npos) {
        if (!BccCheckSumCompareForGGA(str.c_str())) {
          // printf("Check sum pass\n");
          printf("%s", str.c_str());
        }
        retval = 0;
      }
    }
  }

  if (!buffer_line.empty()) {
    buffer_line.clear();
  }
  return retval;
}

int NtripCaster::DealServerConnectRequest(std::vector<std::string> &buffer_line,
                                          const int &sock) {
  char mount_point_name[16] = {0};
  char user_passwd_raw[48] = {0};
  char user[16] = {0};
  char passwd[16] = {0};

  std::string str = buffer_line.back();
  buffer_line.pop_back();
  sscanf(str.c_str(), "%*[^/]%*c%[^ ]", mount_point_name);
  std::string name1 = mount_point_name;
  std::string name2;
  // Check mountpoint.
  if (!mount_point_list_.empty()) {
    auto it = mount_point_list_.begin();
    while (it != mount_point_list_.end()) {
      name2 = it->mount_point_name;
      if (name1 == name2) {
        printf("MountPoint already used!!!\n");
        SendData(sock, "ERROR - Bad Password\r\n", 22);
        // EpollDeregister(epoll_fd_, sock);
        return -1;
      }
      ++it;
    }
  }
  // printf("Can use this mount point\n");
  while (!buffer_line.empty()) {
    str = buffer_line.back();
    buffer_line.pop_back();
    if (str.find("Authorization: Basic") != std::string::npos) {
      sscanf(str.c_str(), "%*[^ ]%*c%*[^ ]%*c%[^\r]", user_passwd_raw);
      if (strlen(user_passwd_raw) > 0) {
        // Decode username && password.
        Base64Decode(user_passwd_raw, user, passwd);
        // printf("Username: [%s], Password: [%s]\n", passwd, passwd);
        // Save the mount point information.
        MountPoint mount_point = {0};
        str = mount_point_name;
        memcpy(mount_point.mount_point_name, str.c_str(), str.size());
        str = user;
        memcpy(mount_point.username, str.c_str(), str.size());
        str = passwd;
        memcpy(mount_point.password, str.c_str(), str.size());
        mount_point.server_fd = sock;
        mount_point_list_.push_back(mount_point);
        // printf("Add mount point ok\n");
        break;
      }
    }
  }
  if (!buffer_line.empty()) {
    str = buffer_line.back();
    buffer_line.pop_back();
    std::string name3;
    // Check mount point information, save to source table list.
    if (str.find("Ntrip-STR:") != std::string::npos) {
      char *str_mnt = new char[16];
      char *str_mnt_check = new char[16];
      sscanf(str.c_str(), "%*[^;]%*c%[^;]%*c%[^;]", str_mnt, str_mnt_check);
      // printf("%s, %s\n", str_mnt, str_mnt_check);
      name2 = str_mnt;
      name3 = str_mnt_check;
      if ((name1 != name2) || (name1 != name3)) {
        SendData(sock, "ERROR - Bad Password\r\n", 22);
        // EpollDeregister(epoll_fd_, sock);
        return -1;
      }
      delete(str_mnt);
      delete(str_mnt_check);
      std::string ntrip_str = str.substr(11, str.size() - 11);
      ntrip_str += "\n";
      ntrip_str_list_.push_back(ntrip_str);
      ntrip_str = "";
      SendData(sock, "ICY 200 OK\r\n", 12);
      return 0;
    }
  }
  return -1;
}

void NtripCaster::SendSourceTableData(const int &sock) {
  std::string ntrip_str = "";
  auto it = ntrip_str_list_.begin();
  while (it != ntrip_str_list_.end()) {
    ntrip_str.append(*it);
    ++it;
  }
  char *datetime = new char[128];
  time_t now;
  time(&now);
  struct tm *tm_now = localtime(&now);
  strftime(datetime, 128, "%x %H:%M:%S %Z", tm_now);
  char *data = new char[MAX_LEN];
  memset(data, 0x0, MAX_LEN);
  int len = snprintf(data, MAX_LEN-1,
                     "SOURCETABLE 200 OK\r\n"
                     "Server: %s\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: %d\r\n"
                     "Date: %s\r\n"
                     "\r\n"
                     "%s"
                     "ENDSOURCETABLE\r\n",
                     kCasterAgent, static_cast<int>(ntrip_str.size()),
                     datetime, ntrip_str.c_str());
  SendData(sock, data, len);
  delete(datetime);
  delete(data);
}

int NtripCaster::DealClientConnectRequest(std::vector<std::string> &buffer_line,
                                          const int &sock) {
  char mount_point_name[16] = {0};
  char user_passwd_raw[48] = {0};
  char user[16] = {0};
  char passwd[16] = {0};

  std::string str = buffer_line.back();
  buffer_line.pop_back();
  sscanf(str.c_str(), "%*[^/]%*c%[^ ]", mount_point_name);
  if (!strlen(mount_point_name)) {  // Request to get the source table.
    SendSourceTableData(sock);
  } else {  // Request to get the differential data.
    if (!mount_point_list_.empty()) {
      std::string name1 = mount_point_name;
      std::string name2;
      auto it = mount_point_list_.begin();
      while (it != mount_point_list_.end()) {
        name2 = it->mount_point_name;
        if (name1 == name2) {
          break;
        }
        ++it;
      }
      if (it == mount_point_list_.end()) {
        printf("MountPoint not find!!!\n");
        SendData(sock, "HTTP/1.1 401 Unauthorized\r\n", 27);
        // EpollDeregister(epoll_fd_, sock);
        return -1;
      }
      while (!buffer_line.empty()) {
        str = buffer_line.back();
        buffer_line.pop_back();
        if (str.find("Authorization: Basic") != std::string::npos) {
          // Get username && password.
          sscanf(str.c_str(), "%*[^ ]%*c%*[^ ]%*c%[^\r]", user_passwd_raw);
          if (strlen(user_passwd_raw) > 0) {
            // Decode username && password.
            Base64Decode(user_passwd_raw, user, passwd);
            // printf("Username: [%s], Password: [%s]\n", passwd, passwd);
            // Check username && password.
            std::string user1 = user;
            std::string user2 = it->username;
            std::string passwd1 = passwd;
            std::string passwd2 = it->password;
            if (user1 != user2 || passwd1 != passwd2) {
              printf("Password error!!!\n");
              SendData(sock, "HTTP/1.1 401 Unauthorized\r\n", 27);
              // EpollDeregister(epoll_fd_, sock);
              return -1;
            }
            SendData(sock, "ICY 200 OK\r\n", 12);
            it->client_socket_list.push_back(sock);
            // printf("Client connect ok\n");
            return 0;
          }
        }
      }
    } else {
      SendData(sock, "HTTP/1.1 401 Unauthorized\r\n", 27);
      // EpollDeregister(epoll_fd_, sock);
    }
  }
  return -1;
}

int NtripCaster::TryToForwardServerData(const int &server_sock,
                                        const char *buf, const int &buf_len) {
  if (!mount_point_list_.empty()) {
    auto it = mount_point_list_.begin();
    while (it != mount_point_list_.end()) {
      if (it->server_fd == server_sock) {
        if (!it->client_socket_list.empty()) {
          auto cit = it->client_socket_list.begin();
          while (cit != it->client_socket_list.end()) {
            SendData(*cit, buf, buf_len);
            //printf("forward %d byte data\n", buf_len);
            ++cit;
          }
        }
        return 0;
      }
     ++it;
    }
  }
  return -1;
}


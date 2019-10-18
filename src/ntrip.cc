#include "ntrip.h"

#include <sys/types.h>                                                                                                                        
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <thread>
#include <list>
#include <vector>


namespace libntrip {

// GPGGA format example.
constexpr char gpgga_buffer[] =
    "$GPGGA,083552.00,3000.0000000,N,11900.0000000,E,"
    "1,08,1.0,0.000,M,100.000,M,,*57\r\n";

// 
// Ntrip util.
//

const char kBase64CodingTable[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int GetIndexByChar(const char &ch) {
  for (int i = 0; i < 64; ++i) {
    if (ch == kBase64CodingTable[i]) {
      return i;
    }
  }
  return -1;
}

inline char GetCharByIndex(const int &index) {
  return kBase64CodingTable[index];
}

int Base64Encode(const char *src, char *result) {
  char temp[3] = {0};
  int i = 0, j = 0, count = 0;
  int len = strlen(src);
  if (len == 0) {
    return -1;
  }

  if (len%3 != 0) {
    count = 3 - len%3;
  }

  while (i < len) {
    strncpy(temp, src+i, 3);
    result[j+0] = GetCharByIndex((temp[0]&0xFC)>>2);
    result[j+1] = GetCharByIndex(((temp[0]&0x3)<<4) | ((temp[1]&0xF0)>>4));
    if (temp[1] == 0) {
      break;
    }
    result[j+2] = GetCharByIndex(((temp[1]&0xF)<<2) | ((temp[2]&0xC0)>>6));
    if (temp[2] == 0) {
      break;
    }
    result[j+3] = GetCharByIndex(temp[2]&0x3F);
    i += 3;
    j += 4;
    memset(temp, 0x0, 3);
  }

  while (count) {
    result[j+4-count] = '=';
    --count;
  }

  return 0;
}

//
// Public method.
//

Ntrip::~Ntrip() {
  if (client_thread_is_running_) {
    ClientStop();
  }
}
   
bool Ntrip::ClientStart(void) {
  int ret = -1;
  char recv_buf[1024] = {0};
  char request_data[1024] = {0};
  char userinfo_raw[48] = {0};
  char userinfo[64] = {0};
  // Generate base64 encoding of username and password.
  snprintf(userinfo_raw, 63 , "%s:%s",
          client_server_user_.c_str(), client_server_passwd_.c_str());
  Base64Encode(userinfo_raw, userinfo);
   // Generate request data format of ntrip.
  snprintf(request_data, 1023,
           "GET /%s HTTP/1.1\r\n"
           "User-Agent: YumingNtripClient1.0\r\n"
           "Accept: */*\r\n"
           "Connection: close\r\n"
           "Authorization: Basic %s\r\n"
           "\r\n",
           client_server_mountpoint_.c_str(), userinfo);

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));                                                                                   
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(client_server_port_);
  server_addr.sin_addr.s_addr = inet_addr(client_server_ip_.c_str());

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Create socket fail\n");
    return false;
  }

  // Connect to caster.
  if (connect(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
              sizeof(struct sockaddr_in)) < 0) {
    printf("Connect caster failed!!!\n");
    return false;
  }

  int flags = fcntl(socket_fd, F_GETFL);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

  // Send request data.
  if (send(socket_fd, request_data, strlen(request_data), 0) < 0) {
    printf("send request failed!!!\n");
    close(socket_fd);
    return false;
  }

  // Waitting for request to connect caster success.
  int timeout = 3;
  while (timeout--) {
    ret = recv(socket_fd, recv_buf, sizeof(recv_buf), 0);
    if ((ret > 0) && !strncmp(recv_buf, "ICY 200 OK\r\n", 12)) {
      ret = send(socket_fd, gpgga_buffer, strlen(gpgga_buffer), 0);
      if (ret < 0) {
        printf("Send gpgga data fail\n");
        close(socket_fd);
        return false;
      }
      // printf("Send gpgga data ok\n");
      break;
    } else if (ret == 0) {
      printf("Remote socket close!!!\n");
      close(socket_fd);
      return false;
    }
    sleep(1);
  }

  if (timeout <= 0) {
    return false;
  }

  client_socket_ = socket_fd;
  client_thread_ = std::thread(&Ntrip::ClientTheradHandler, this);
  printf("Client starting ...\n");
  return true;
}

void Ntrip::ClientStop(void) {
  client_thread_is_running_ = false;
  client_thread_.join();
  if (client_socket_ != -1) {
    close(client_socket_);
    client_socket_ = -1;
  }
  if (!client_buffer_list_.empty()) {
    client_buffer_list_.clear();
  }
}

//
// Private method.
//

// Clinent hread handler.
void Ntrip::ClientTheradHandler(void) {
  int ret;
  char recv_buffer[1024] = {};
  client_thread_is_running_ = true;
  while (client_thread_is_running_) {
    memset(recv_buffer, 0x0, sizeof(recv_buffer));
    ret = recv(client_socket_, recv_buffer, sizeof(recv_buffer), 0);
    if (ret == 0) {
      printf("remote socket close!!!\n");
      break;
    } else if (ret > 0) {
      std::vector<char> buffer(recv_buffer, recv_buffer + ret);
      client_buffer_list_.push_back(buffer);
    }
  }
  close(client_socket_);
  client_socket_ = -1;
  client_thread_is_running_ = false;
}

}  // namespace libntrip

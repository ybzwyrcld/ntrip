// Copyright 2019 Yuming Meng. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ntrip_server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <thread>  // NOLINT.
#include <list>
#include <vector>

#include "ntrip_util.h"


namespace libntrip {

// RTK format example.
constexpr uint8_t example_data[] = {
    0xd3, 0x00, 0x70, 0x8e, 0x43, 0x56, 0x45, 0x00, 0x00,
    0x55, 0xfb, 0x89, 0xff, 0xff, '\r', '\n'
};

//
// Public method.
//

NtripServer::~NtripServer() {
  if (thread_is_running_) {
    Stop();
  }
}

bool NtripServer::Run(void) {
  int ret = -1;
  char request_buffer[1024] = {0};
  char userinfo_raw[48] = {0};
  char userinfo[64] = {0};
  // Generate base64 encoding of username and password.
  snprintf(userinfo_raw, sizeof(userinfo_raw) , "%s:%s",
           user_.c_str(), passwd_.c_str());
  Base64Encode(userinfo_raw, userinfo);
  // Generate request data format of ntrip.
  snprintf(request_buffer, sizeof(request_buffer),
           "POST /%s HTTP/1.1\r\n"
           "Host: %s:%d\r\n"
           "Ntrip-Version: Ntrip/2.0\r\n"
           "User-Agent: %s\r\n"
           "Authorization: Basic %s\r\n"
           "Ntrip-STR: %s\r\n"
           "Connection: close\r\n"
           "Transfer-Encoding: chunked\r\n",
           mountpoint_.c_str(), server_ip_.c_str(), server_port_,
           kServerAgent, userinfo, ntrip_str_.c_str());

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port_);
  server_addr.sin_addr.s_addr = inet_addr(server_ip_.c_str());

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Create socket fail\n");
    return false;
  }

  // Connect to caster.
  if (connect(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
              sizeof(struct sockaddr_in)) < 0) {
    printf("Connect remote server failed!!!\n");
    close(socket_fd);
    return false;
  }

  int flags = fcntl(socket_fd, F_GETFL);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

  // Send request data.
  if (send(socket_fd, request_buffer, strlen(request_buffer), 0) < 0) {
    printf("Send authentication request failed!!!\n");
    close(socket_fd);
    return false;
  }

  // Waitting for request to connect caster success.
  int timeout = 3;
  while (timeout--) {
    memset(request_buffer, 0x0, sizeof(request_buffer));
    ret = recv(socket_fd, request_buffer, sizeof(request_buffer), 0);
    if ((ret > 0) && !strncmp(request_buffer, "ICY 200 OK\r\n", 12)) {
      // printf("Connect to caster success\n");
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
  // TCP socket keepalive.
  int keepalive = 1;  // Enable keepalive attributes.
  int keepidle = 30;  // Time out for starting detection.
  int keepinterval = 5;  // Time interval for sending packets during detection.
  int keepcount = 3;  // Max times for sending packets during detection.
  setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive,
             sizeof(keepalive));
  setsockopt(socket_fd, SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
  setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL, &keepinterval,
             sizeof(keepinterval));
  setsockopt(socket_fd, SOL_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount));
  socket_fd_ = socket_fd;
  thread_ = std::thread(&NtripServer::TheradHandler, this);
  thread_.detach();
  service_is_running_ = true;
  printf("NtripServer starting ...\n");
  return true;
}

void NtripServer::Stop(void) {
  thread_is_running_ = false;
  service_is_running_ = false;
  if (socket_fd_ != -1) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
  if (!data_list_.empty()) {
    data_list_.clear();
  }
}

//
// Private method.
//

void NtripServer::TheradHandler(void) {
  int ret;
  char recv_buffer[1024] = {};
  thread_is_running_ = true;
  int cnt = 100;
  while (thread_is_running_) {
    // TODO(mengyuming@hotmail.com) : Now just send test data.
    if (--cnt == 0) {  // Near once per second.
      ret = send(socket_fd_, example_data, sizeof(example_data), 0);
      if (ret > 0) {
        printf("Send example_data success\n");
      } else if (ret < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)) {
          continue;
        } else {
          printf("Remote socket error!!!\n");
          break;
        }
      } else if (ret == 0) {
        printf("Remote socket close!!!\n");
        break;
      }
      cnt = 100;
    }
    memset(recv_buffer, 0x0, sizeof(recv_buffer));
    ret = recv(socket_fd_, recv_buffer, sizeof(recv_buffer), 0);
    if (ret == 0) {
      printf("Remote socket close!!!\n");
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  close(socket_fd_);
  socket_fd_ = -1;
  thread_is_running_ = false;
  service_is_running_ = false;
}

}  // namespace libntrip

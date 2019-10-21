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

#include "ntrip_client.h"

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
#include <thread>  // NOLINT.
#include <list>
#include <vector>

#include "ntrip_util.h"


namespace libntrip {

// GPGGA format example.
constexpr char gpgga_buffer[] =
    "$GPGGA,083552.00,3000.0000000,N,11900.0000000,E,"
    "1,08,1.0,0.000,M,100.000,M,,*57\r\n";

//
// Public method.
//

NtripClient::~NtripClient() {
  if (thread_is_running_) {
    Stop();
  }
}

bool NtripClient::Run(void) {
  int ret = -1;
  char recv_buf[1024] = {0};
  char request_data[1024] = {0};
  char userinfo_raw[48] = {0};
  char userinfo[64] = {0};
  // Generate base64 encoding of username and password.
  snprintf(userinfo_raw, sizeof(userinfo_raw) , "%s:%s",
           user_.c_str(), passwd_.c_str());
  Base64Encode(userinfo_raw, userinfo);
  // Generate request data format of ntrip.
  snprintf(request_data, sizeof(request_data),
           "GET /%s HTTP/1.1\r\n"
           "User-Agent: %s\r\n"
           "Accept: */*\r\n"
           "Connection: close\r\n"
           "Authorization: Basic %s\r\n"
           "\r\n",
           mountpoint_.c_str(), kClientAgent, userinfo);

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
    printf("Connect caster failed!!!\n");
    return false;
  }

  int flags = fcntl(socket_fd, F_GETFL);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

  // Send request data.
  if (send(socket_fd, request_data, strlen(request_data), 0) < 0) {
    printf("Send request failed!!!\n");
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

  socket_fd_ = socket_fd;
  thread_ = std::thread(&NtripClient::TheradHandler, this);
  thread_.detach();
  service_is_running_ = true;
  printf("NtripClient service starting ...\n");
  return true;
}

void NtripClient::Stop(void) {
  service_is_running_ = false;
  thread_is_running_ = false;
  if (socket_fd_ != -1) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
  if (!buffer_list_.empty()) {
    buffer_list_.clear();
  }
}

//
// Private method.
//

void NtripClient::TheradHandler(void) {
  int ret;
  char recv_buffer[1024] = {};
  thread_is_running_ = true;
  while (thread_is_running_) {
    memset(recv_buffer, 0x0, sizeof(recv_buffer));
    ret = recv(socket_fd_, recv_buffer, sizeof(recv_buffer), 0);
    if (ret == 0) {
      printf("Remote socket close!!!\n");
      break;
    } else if (ret < 0) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)) {
        continue;
      } else {
        printf("Remote socket error!!!\n");
        break;
      }
    } else if (ret > 0) {
      std::vector<char> buffer(recv_buffer, recv_buffer + ret);
      buffer_list_.push_back(buffer);
    }
  }
  close(socket_fd_);
  socket_fd_ = -1;
  thread_is_running_ = false;
  service_is_running_ = false;
}

}  // namespace libntrip

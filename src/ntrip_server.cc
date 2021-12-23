// MIT License
//
// Copyright (c) 2021 Yuming Meng
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "ntrip/ntrip_server.h"

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

#include "ntrip/ntrip_util.h"


namespace libntrip {

namespace {

constexpr int kBufferSize = 1024;

}  // namespace

//
// Public method.
//

NtripServer::~NtripServer() {
  Stop();
}

bool NtripServer::Run(void) {
  if (service_is_running_.load()) return true;
  Stop();
  int ret = -1;
  char request_buffer[kBufferSize] = {0};
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
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port_);
  server_addr.sin_addr.s_addr = inet_addr(server_ip_.c_str());

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Create socket failed, errno = -%d\n", errno);
    return false;
  }

  // Connect to caster.
  if (connect(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
      sizeof(server_addr)) < 0) {
    printf("Connect to NtripCaster[%s:%d] failed, errno = -%d\n",
        server_ip_.c_str(), server_port_, errno);
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
    ret = recv(socket_fd, request_buffer, kBufferSize, 0);
    if (ret > 0) {
      std::string result(request_buffer, ret);
      if ((result.find("HTTP/1.1 200 OK") != std::string::npos) ||
          (result.find("ICY 200 OK") != std::string::npos)) {
        // printf("Connect to caster success\n");
        break;
      }
    } else if (ret == 0) {
      printf("Remote socket close!!!\n");
      close(socket_fd);
      return false;
    }
    sleep(1);
  }

  if (timeout <= 0) {
    printf("NtripCaster[%s:%d %s %s %s] access failed!!!\n",
        server_ip_.c_str(), server_port_,
        user_.c_str(), passwd_.c_str(), mountpoint_.c_str());
    close(socket_fd);
    return false;
  }
  // TCP socket keepalive.
  int keepalive = 1;  // Enable keepalive attributes.
  int keepidle = 30;  // Time out for starting detection.
  int keepinterval = 5;  // Time interval for sending packets during detection.
  int keepcount = 3;  // Max times for sending packets during detection.
  setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE,
      &keepalive, sizeof(keepalive));
  setsockopt(socket_fd, SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
  setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL,
      &keepinterval, sizeof(keepinterval));
  setsockopt(socket_fd, SOL_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount));
  socket_fd_ = socket_fd;
  thread_ = std::thread(&NtripServer::TheradHandler, this);
  // Thread may not start immediately.
  service_is_running_.store(true);
  return true;
}

void NtripServer::Stop(void) {
  service_is_running_.store(false);
  if (socket_fd_ > 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
  if (!data_list_.empty()) {
    data_list_.clear();
  }
  if (thread_.joinable()) thread_.join();
}

//
// Private method.
//

void NtripServer::TheradHandler(void) {
  service_is_running_.store(true);
  int ret;
  char recv_buffer[kBufferSize] = {};
  printf("NtripServer service running...\n");
  while (service_is_running_.load()) {
    ret = recv(socket_fd_, recv_buffer, kBufferSize, 0);
    if (ret == 0) {
      printf("Remote socket closed!!!\n");
      break;
    } else if (ret < 0) {
      if ((errno != EAGAIN) && (errno != EWOULDBLOCK) && (errno != EINTR)) {
        printf("Remote socket error!!!\n");
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  printf("NtripServer service done.\n");
  service_is_running_.store(false);
}

}  // namespace libntrip

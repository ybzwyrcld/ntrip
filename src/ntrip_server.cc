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

constexpr int kBufferSize = 4096;

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
  // Establish a connection with NtripCaster.
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
  if (connect(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
      sizeof(server_addr)) < 0) {
    printf("Connect to NtripCaster[%s:%d] failed, errno = -%d\n",
        server_ip_.c_str(), server_port_, errno);
    close(socket_fd);
    return false;
  }
  // Set non-blocking.
  int flags = fcntl(socket_fd, F_GETFL);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
  // Ntrip connection authentication.
  int ret = -1;
  char userinfo[128] = {0};
  char userinfo_base64[256] = {0};
  std::unique_ptr<char[]> buffer(
      new char[kBufferSize], std::default_delete<char[]>());
  // Generate base64 encoding of username and password.
  snprintf(userinfo, sizeof(userinfo) , "%s:%s",
      user_.c_str(), passwd_.c_str());
  Base64Encode(userinfo, userinfo_base64);
  // Generate request data format of ntrip.
  ret = snprintf(buffer.get(), kBufferSize-1,
      "POST /%s HTTP/1.1\r\n"
      "Host: %s:%d\r\n"
      "Ntrip-Version: Ntrip/2.0\r\n"
      "User-Agent: %s\r\n"
      "Authorization: Basic %s\r\n"
      "Ntrip-STR: %s\r\n"
      "Connection: close\r\n"
      "Transfer-Encoding: chunked\r\n",
      mountpoint_.c_str(), server_ip_.c_str(), server_port_,
      kServerAgent, userinfo_base64, ntrip_str_.c_str());
  if (send(socket_fd, buffer.get(), ret, 0) < 0) {
    printf("Send authentication request failed!!!\n");
    close(socket_fd);
    return false;
  }
  // Waitting for request to connect caster success.
  int timeout = 3;
  while (timeout--) {
    ret = recv(socket_fd, buffer.get(), kBufferSize, 0);
    if (ret > 0) {
      std::string result(buffer.get(), ret);
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
  thread_.reset(&NtripServer::ThreadHandler, this);
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
  thread_.join();
}

//
// Private method.
//

void NtripServer::ThreadHandler(void) {
  service_is_running_.store(true);
  int ret;
  std::unique_ptr<char[]> buffer(
      new char[kBufferSize], std::default_delete<char[]>());
  printf("NtripServer service running...\n");
  while (service_is_running_.load()) {
    ret = recv(socket_fd_, buffer.get(), kBufferSize, 0);
    if (ret == 0) {
      printf("Remote socket closed!!!\n");
      break;
    } else if (ret < 0) {
      if ((errno != EAGAIN) && (errno != EWOULDBLOCK) && (errno != EINTR)) {
        printf("Remote socket error!!!\n");
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  printf("NtripServer service done.\n");
  service_is_running_.store(false);
}

}  // namespace libntrip

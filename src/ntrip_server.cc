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

#if defined(__linux__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif  // defined(__linux__)
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>

#include "ntrip/ntrip_util.h"
#include "cmake_definition.h"


#if defined(__linux__)
#define ENABLE_TCP_KEEPALIVE
#endif // defined(__linux__)

namespace libntrip {

namespace {

using socket_t = decltype(socket(AF_INET, SOCK_STREAM, 0));

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
  socket_t socket_fd;
  // Establish a connection with NtripCaster.
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port_);
  server_addr.sin_addr.s_addr = inet_addr(server_ip_.c_str());
#if defined(WIN32) || defined(_WIN32)
  WSADATA ws_data;
  if (WSAStartup(MAKEWORD(2,2), &ws_data) != 0) {
    return false;
  }
  socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_fd == INVALID_SOCKET) {
    printf("Create socket failed!\n");
    WSACleanup();
    return false;
  }
  server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip_.c_str());
#else
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Create socket failed, errno = -%d\n", errno);
    return false;
  }
#endif  // defined(WIN32) || defined(_WIN32)
  if (connect(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
      sizeof(server_addr)) < 0) {
    printf("Connect to NtripCaster[%s:%d] failed, errno = -%d\n",
        server_ip_.c_str(), server_port_, errno);
#if defined(WIN32) || defined(_WIN32)
    closesocket(socket_fd);
    WSACleanup();
#else
    close(socket_fd);
#endif  // defined(WIN32) || defined(_WIN32)
    return false;
  }
  // Set non-blocking.
#if defined(WIN32) || defined(_WIN32)
  unsigned long ul = 1;
  if (ioctlsocket(socket_fd, FIONBIO, &ul) == SOCKET_ERROR) {
    closesocket(socket_fd);
    WSACleanup();
    return false;
  }
#else
  int flags = fcntl(socket_fd, F_GETFL);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
#endif  // defined(WIN32) || defined(_WIN32)
  // Ntrip connection authentication.
  int ret = -1;
  std::string user_passwd = user_ + ":" + passwd_;
  std::string user_passwd_base64;
  std::unique_ptr<char[]> buffer(
      new char[kBufferSize], std::default_delete<char[]>());
  // Generate base64 encoding of username and password.
  Base64Encode(user_passwd, &user_passwd_base64);
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
      kServerAgent, user_passwd_base64.c_str(), ntrip_str_.c_str());
  if (send(socket_fd, buffer.get(), ret, 0) < 0) {
    printf("Send authentication request failed!!!\n");
#if defined(WIN32) || defined(_WIN32)
    closesocket(socket_fd);
    WSACleanup();
#else
    close(socket_fd);
#endif  // defined(WIN32) || defined(_WIN32)
    return false;
  }
  // Waitting for request to connect caster success.
  int timeout = 30;  // 30*100ms=3s.
  while (timeout--) {
    ret = recv(socket_fd, buffer.get(), kBufferSize, 0);
    if (ret > 0) {
      std::string result(buffer.get(), ret);
      if ((result.find("HTTP/1.1 200 OK") != std::string::npos) ||
          (result.find("ICY 200 OK") != std::string::npos)) {
        // printf("Connect to caster success\n");
        break;
      } else {
        printf("Request result: %s\n", result.c_str());
      }
    } else if (ret == 0) {
      printf("Remote socket close!!!\n");
#if defined(WIN32) || defined(_WIN32)
      closesocket(socket_fd);
      WSACleanup();
#else
      close(socket_fd);
#endif  // defined(WIN32) || defined(_WIN32)
      return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  if (timeout <= 0) {
    printf("NtripCaster[%s:%d %s %s %s] access failed!!!\n",
        server_ip_.c_str(), server_port_,
        user_.c_str(), passwd_.c_str(), mountpoint_.c_str());
#if defined(WIN32) || defined(_WIN32)
    closesocket(socket_fd);
    WSACleanup();
#else
    close(socket_fd);
#endif  // defined(WIN32) || defined(_WIN32)
    return false;
  }
#if defined(ENABLE_TCP_KEEPALIVE)
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
#endif  // defined(ENABLE_TCP_KEEPALIVE)
  socket_fd_ = socket_fd;
  thread_.reset(&NtripServer::ThreadHandler, this);
  return true;
}

int NtripServer::SendData(const char *data, int size) {
  return (size == send(socket_fd_, data, size, 0))-1;
}

void NtripServer::Stop(void) {
  service_is_running_.store(false);
#if defined(WIN32) || defined(_WIN32)
  if (socket_fd_ != INVALID_SOCKET) {
    closesocket(socket_fd_);
    WSACleanup();
    socket_fd_ = INVALID_SOCKET;
  }
#else
  if (socket_fd_ > 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
#endif  // defined(WIN32) || defined(_WIN32)
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
      if ((errno != 0) && (errno != EAGAIN) &&
          (errno != EWOULDBLOCK) && (errno != EINTR)) {
        printf("Remote socket error!!!\n");
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
#if defined(WIN32) || defined(_WIN32)
  if (socket_fd_ != INVALID_SOCKET) {
    closesocket(socket_fd_);
    WSACleanup();
    socket_fd_ = INVALID_SOCKET;
  }
#else
  if (socket_fd_ > 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
#endif  // defined(WIN32) || defined(_WIN32)
  printf("NtripServer service done.\n");
  service_is_running_.store(false);
}

}  // namespace libntrip

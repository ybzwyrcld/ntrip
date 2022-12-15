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

#ifndef NTRIPLIB_NTRIP_SERVER_H_
#define NTRIPLIB_NTRIP_SERVER_H_

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif  // defined(WIN32) || defined(_WIN32)

#include <string>
#include <thread>  // NOLINT.
#include <list>
#include <vector>
#include <atomic>

#include "./thread_raii.h"


namespace libntrip {

class NtripServer {
 public:
  NtripServer() = default;
  NtripServer(NtripServer const&) = delete;
  NtripServer(NtripServer&&) = delete;
  NtripServer& operator=(NtripServer const&) = delete;
  NtripServer& operator=(NtripServer&&) = delete;
  NtripServer(std::string const& ip, int port,
      std::string const& user, std::string const& passwd,
      std::string const& mountpoint, std::string const& ntrip_str) :
          server_ip_(ip), server_port_(port),
          user_(user), passwd_(passwd),
          mountpoint_(mountpoint), ntrip_str_(ntrip_str) { }
  ~NtripServer();

  void Init(std::string const& ip, int port,
      std::string const& user, std::string const& passwd,
      std::string const& mountpoint, std::string const& ntrip_str) {
    server_ip_ = ip;
    server_port_ = port;
    user_ = user;
    passwd_ = passwd;
    mountpoint_ = mountpoint;
    ntrip_str_ = ntrip_str;
  }

  // Return 0 if success.
  int SendData(const char *data, int size);
  int SendData(std::vector<char> const& data) {
    return SendData(data.data(), data.size());
  }
  int SendData(std::string const& data) {
    return SendData(data.data(), data.size());
  }

  // TODO(mengyuming@hotmail.com) : Not implemented.
  void PushData(char const* data, int size);
  void PushData(std::vector<char> const& data);
  void PushData(std::string const& data);

  bool Run(void);
  void Stop(void);
  bool service_is_running(void) const {
    return service_is_running_.load();
  }

 private:
  // Thread handler.
  void ThreadHandler(void);

  std::atomic_bool service_is_running_ = {false};
  std::string server_ip_;
  int server_port_ = 8090;
  std::string user_;
  std::string passwd_;
  std::string mountpoint_;
  std::string ntrip_str_;
#if defined(WIN32) || defined(_WIN32)
  SOCKET socket_fd_ = INVALID_SOCKET;
#else
  int socket_fd_ = -1;
#endif  // defined(WIN32) || defined(_WIN32)
  Thread thread_;
  std::list<std::vector<char>> data_list_;
};

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_SERVER_H_

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


#include <sys/types.h>
#include <sys/socket.h>

#include <string>
#include <thread>  // NOLINT.
#include <list>
#include <vector>
#include <atomic>


namespace libntrip {

class NtripServer {
 public:
  NtripServer() = default;
  NtripServer(const NtripServer &) = delete;
  NtripServer(NtripServer&&) = delete;
  NtripServer& operator=(const NtripServer &) = delete;
  NtripServer& operator=(NtripServer&&) = delete;
  NtripServer(const std::string &ip, const int &port,
         const std::string &user, const std::string &passwd,
         const std::string &mountpoint, const std::string &ntrip_str) :
      server_ip_(ip), server_port_(port),
      user_(user), passwd_(passwd),
      mountpoint_(mountpoint), ntrip_str_(ntrip_str) { }
  ~NtripServer();

  void Init(const std::string &ip, const int &port,
            const std::string &user, const std::string &passwd,
            const std::string &mountpoint, const std::string &ntrip_str) {
    server_ip_ = ip;
    server_port_ = port;
    user_ = user;
    passwd_ = passwd;
    mountpoint_ = mountpoint;
    ntrip_str_ = ntrip_str;
  }

  // Return 0 if success.
  int SendData(const char *data, const int &size) {
    return (size == send(socket_fd_, data, size, 0))-1;
  }
  int SendData(const std::vector<char> &data) {
    return SendData(data.data(), data.size());
  }
  int SendData(const std::string &data) {
    return SendData(data.data(), data.size());
  }

  // TODO(mengyuming@hotmail.com) : Not implemented.
  void PushData(const char *data, const int &size);
  void PushData(const std::vector<char> &data);
  void PushData(const std::string &data);

  bool Run(void);
  void Stop(void);
  bool service_is_running(void) const {
    return service_is_running_.load();
  }

 private:
  // Thread handler.
  void TheradHandler(void);

  std::atomic_bool service_is_running_ = {false};
  std::thread thread_;
  std::string server_ip_;
  int server_port_ = -1;
  std::string user_;
  std::string passwd_;
  std::string mountpoint_;
  std::string ntrip_str_;
  int socket_fd_ = -1;
  std::list<std::vector<char>> data_list_;
};

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_SERVER_H_

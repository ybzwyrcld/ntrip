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

#ifndef NTRIPLIB_NTRIP_SERVER_H_
#define NTRIPLIB_NTRIP_SERVER_H_


#include <sys/types.h>
#include <sys/socket.h>

#include <string>
#include <thread>  // NOLINT.
#include <list>
#include <vector>

namespace libntrip {

class NtripServer {
 public:
  NtripServer() = default;
  NtripServer(const NtripServer &) = delete;
  NtripServer& operator=(const NtripServer &) = delete;
  NtripServer(const std::string &ip, const int &port,
         const std::string &user, const std::string &passwd,
         const std::string &mountpoint,
         const std::string &ntrip_str) :
      server_ip_(ip),
      server_port_(port),
      user_(user),
      passwd_(passwd),
      mountpoint_(mountpoint),
      ntrip_str_(ntrip_str) { }
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

  bool SendData(const char *data, const int &size) {
    return (size == send(socket_fd_, data, size, 0));
  }
  bool SendData(const std::vector<char> &data) {
    return SendData(data.data(), data.size());
  }
  bool SendData(const std::string &data) {
    return SendData(data.data(), data.size());
  }

  // TODO(mengyuming@hotmail.com) : Not implemented.
  void PushData(const char *data, const int &size);
  void PushData(const std::vector<char> &data);
  void PushData(const std::string &data);

  bool Run(void);
  void Stop(void);
  bool service_is_running(void) const {
    return service_is_running_;
  }

 private:
  // Thread handler.
  void TheradHandler(void);

  bool thread_is_running_ = false;
  bool service_is_running_ = false;
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

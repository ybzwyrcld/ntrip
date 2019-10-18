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

#ifndef NTRIPLIB_CLIENT_H_
#define NTRIPLIB_CLIENT_H_


#include <string>
#include <thread>  // NOLINT.
#include <list>
#include <vector>

namespace libntrip {

class Client {
 public:
  Client() = default;
  Client(const Client &) = delete;
  Client& operator=(const Client &) = delete;
  Client(const std::string &ip, const int &port,
         const std::string &user, const std::string &passwd,
         const std::string &mountpoint) :
      server_ip_(ip),
      server_port_(port),
      server_user_(user),
      server_passwd_(passwd),
      server_mountpoint_(mountpoint) { }
  ~Client();

  void Init(const std::string &ip, const int &port,
      const std::string &user, const std::string &passwd,
      const std::string &mountpoint) {
    server_ip_ = ip;
    server_port_ = port;
    server_user_ = user;
    server_passwd_ = passwd;
    server_mountpoint_ = mountpoint;
  }

  bool BufferEmpty(void) const {
    return buffer_list_.empty();
  }
  bool Buffer(std::vector<char> *buffer) {
    if (BufferEmpty() || buffer == nullptr) return false;
    buffer->clear();
    buffer->assign(buffer_list_.front().begin(),
        buffer_list_.front().end());
    buffer_list_.pop_front();
    return true;
  }

  bool Start(void);
  void Stop(void);

 private:
  // Thread handler.
  void TheradHandler(void);

  bool thread_is_running_ = false;
  std::thread thread_;
  std::string server_ip_;
  int server_port_ = -1;
  std::string server_user_;
  std::string server_passwd_;
  std::string server_mountpoint_;
  int socket_ = -1;
  std::list<std::vector<char>> buffer_list_;
};

}  // namespace libntrip

#endif  // NTRIPLIB_CLIENT_H_

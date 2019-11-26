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

#ifndef NTRIPLIB_NTRIP_CLIENT_H_
#define NTRIPLIB_NTRIP_CLIENT_H_

#include <string>
#include <thread>  // NOLINT.
#include <functional>

namespace libntrip {

using ClientCallback = std::function<void(const char *, const int &)>;

class NtripClient {
 public:
  NtripClient() = default;
  NtripClient(const NtripClient &) = delete;
  NtripClient& operator=(const NtripClient &) = delete;
  NtripClient(const std::string &ip, const int &port,
              const std::string &user, const std::string &passwd,
              const std::string &mountpoint) :
      server_ip_(ip),
      server_port_(port),
      user_(user),
      passwd_(passwd),
      mountpoint_(mountpoint) { }
  ~NtripClient();

  void Init(const std::string &ip, const int &port,
            const std::string &user, const std::string &passwd,
            const std::string &mountpoint) {
    server_ip_ = ip;
    server_port_ = port;
    user_ = user;
    passwd_ = passwd;
    mountpoint_ = mountpoint;
  }
  // 设置发送的GGA语句.
  // 根据ntrip账号的要求, 如果距离服务器位置过远, 服务器不会返回差分数据.
  void set_gga_buffer(const std::string &gga_buffer) {
    gga_buffer_ = gga_buffer;
  }
  // 设置接收到数据时的回调函数.
  void OnReceived(const ClientCallback &callback) { callback_ = callback; }
  bool Run(void);
  void Stop(void);
  bool service_is_running(void) const {
    return service_is_running_;
  }

 private:
  // Thread handler.
  void TheradHandler(void);

  bool service_is_running_ = false;
  bool thread_is_running_ = false;
  std::thread thread_;
  std::string server_ip_;
  int server_port_ = -1;
  std::string user_;
  std::string passwd_;
  std::string mountpoint_;
  std::string gga_buffer_;
  int socket_fd_ = -1;
  ClientCallback callback_;
};

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_CLIENT_H_

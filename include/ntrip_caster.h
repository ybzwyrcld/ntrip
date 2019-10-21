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

#ifndef NTRIPLIB_NTRIP_CASTER_H_
#define NTRIPLIB_NTRIP_CASTER_H_

#include <sys/epoll.h>

#include <string>
#include <list>
#include <vector>
#include <thread>  // NOLINT.

#include "mount_point.h"


namespace libntrip {

class NtripCaster {
 public:
  NtripCaster() = default;
  NtripCaster(const NtripCaster &) = delete;
  NtripCaster& operator=(const NtripCaster&) = delete;
  ~NtripCaster();

  void Init(const int &port, const int &sock_count, const int &time_out) {
    server_port_ = port;
    max_count_ = sock_count;
    time_out_ = time_out;
  }
  void Init(const std::string &server_ip, const int &port,
            const int &sock_count, const int &time_out) {
    server_ip_ = server_ip;
    server_port_ = port;
    max_count_ = sock_count;
    time_out_ = time_out;
  }
  bool Run(void);
  void Stop(void);
  bool service_is_running(void) const {
    return service_is_running_;
  }

 private:
  void ThreadHandler(void);
  int NtripCasterWait(const int &time_out);
  int AcceptNewConnect(void);
  int RecvData(const int &sock, char *recv_buf);
  int SendData(const int &sock, const char *send_buf, const int &buf_len);
  void DealDisconnect(const int &sock);
  int ParseData(const int &sock, char *recv_buf, const int &len);
  int DealServerConnectRequest(std::vector<std::string> *buffer_line,
                               const int &sock);
  void SendSourceTableData(const int &sock);
  int DealClientConnectRequest(std::vector<std::string> *buffer_line,
                               const int &sock);
  int TryToForwardServerData(const int &server_sock,
                             const char *buf, const int &buf_len);

  bool main_thread_is_running_ = false;
  bool service_is_running_ = false;
  std::string server_ip_;
  int server_port_ = -1;
  int time_out_ = 0;
  int listen_sock_ = -1;
  int epoll_fd_ = -1;
  int max_count_ = 0;
  struct epoll_event *epoll_events_ = nullptr;
  std::thread main_thread_;
  std::list<MountPoint> mount_point_list_;
  std::vector<std::string> ntrip_str_list_;
};

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_CASTER_H_

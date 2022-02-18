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

#ifndef NTRIPLIB_NTRIP_CASTER_H_
#define NTRIPLIB_NTRIP_CASTER_H_

#include <sys/epoll.h>

#include <atomic>
#include <string>
#include <list>
#include <vector>
#include <thread>  // NOLINT.

#include "mount_point.h"
#include "../thread_raii.h"


namespace libntrip {

class NtripCaster {
 public:
  NtripCaster() = default;
  NtripCaster(NtripCaster const&) = delete;
  NtripCaster(NtripCaster&&) = delete;
  NtripCaster& operator=(NtripCaster const&) = delete;
  NtripCaster& operator=(NtripCaster&&) = delete;
  ~NtripCaster();

  void Init(int server_port, int max_connection_count,
      int epoll_wait_timeout) {
    server_port_ = server_port;
    max_count_ = max_connection_count;
    time_out_ = epoll_wait_timeout;
  }
  void Init(std::string const& server_ip, int server_port,
      int max_connection_count, int epoll_wait_timeout) {
    server_ip_ = server_ip;
    server_port_ = server_port;
    max_count_ = max_connection_count;
    time_out_ = epoll_wait_timeout;
  }
  bool Run(void);
  void Stop(void);
  bool service_is_running(void) const {
    return service_is_running_.load();
  }

 private:
  void ThreadHandler(void);
  int AcceptNewConnect(void);
  void Disconnect(int socket_fd);
  int ParseData(int socket_fd, char const* buffer, int buffer_len);
  void SendSourceTableData(int socket_fd);
  int TryToForwardServerData(int socket_fd,
      char const* buffer, int buffer_len);
  int ServerConnectRequest(
      std::vector<std::string> const& lines, int socket_fd);
  int ClientConnectRequest(
      std::vector<std::string> const& lines, int socket_fd);

  std::atomic_bool service_is_running_ = {false};
  std::string server_ip_;
  int server_port_ = -1;
  int time_out_ = 0;
  int listen_sock_ = -1;
  int epoll_fd_ = -1;
  int max_count_ = 0;
  struct epoll_event *epoll_events_ = nullptr;
  Thread thread_;
  std::list<MountPointInformation> mount_point_infos_;
  std::vector<std::string> ntrip_str_list_;
};

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_CASTER_H_

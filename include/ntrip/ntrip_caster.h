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
    service_is_running_ = false;
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

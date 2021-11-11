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

#ifndef NTRIPLIB_NTRIP_CLIENT_H_
#define NTRIPLIB_NTRIP_CLIENT_H_

#include <atomic>
#include <string>
#include <thread>  // NOLINT.
#include <functional>

namespace libntrip {

using ClientCallback = std::function<void (char const* _buffer, int _size)>;

class NtripClient {
 public:
  NtripClient() = default;
  NtripClient(const NtripClient &) = delete;
  NtripClient(NtripClient&&) = delete;
  NtripClient& operator=(const NtripClient &) = delete;
  NtripClient& operator=(NtripClient&&) = delete;
  NtripClient(const std::string &ip, const int &port,
              const std::string &user, const std::string &passwd,
              const std::string &mountpoint) :
      server_ip_(ip), server_port_(port),
      user_(user), passwd_(passwd),
      mountpoint_(mountpoint) { }
  ~NtripClient() { Stop(); }

  void Init(const std::string &ip, const int &port,
            const std::string &user, const std::string &passwd,
            const std::string &mountpoint) {
    server_ip_ = ip;
    server_port_ = port;
    user_ = user;
    passwd_ = passwd;
    mountpoint_ = mountpoint;
  }
  // 更新发送的GGA语句.
  // 根据ntrip账号的要求, 如果距离服务器位置过远, 服务器不会返回差分数据.
  void set_gga_buffer(const std::string &gga_buffer) {
    gga_buffer_ = gga_buffer;
    gga_is_update_.store(true);
  }
  // 设置固定位置坐标, 由此自动生成GGA数据.
  void set_location(double const& latitude, double const& longitude) {
    latitude_ = latitude;
    longitude_ = longitude;
  }
  // 设置GGA上报时间间隔, 单位秒(s).
  void set_report_interval(int const& intv) {
    report_interval_ = intv;
  }

  // 设置接收到数据时的回调函数.
  void OnReceived(const ClientCallback &callback) { callback_ = callback; }
  bool Run(void);
  void Stop(void);
  bool service_is_running(void) const {
    return service_is_running_.load();
  }

 private:
  // Thread handler.
  void TheradHandler(void);

  std::atomic_bool service_is_running_ = {false};
  std::atomic_bool gga_is_update_ = {false};  // 外部更新GGA数据标志.
  int report_interval_;  // GGA数据上报时间间隔.
  double latitude_ = 22.570535;  // 固定坐标纬度.
  double longitude_ = 113.937739;  // 固定坐标经度.
  std::thread thread_;
  std::string server_ip_;
  int server_port_ = -1;
  std::string user_;
  std::string passwd_;
  std::string mountpoint_;
  std::string gga_buffer_;
  int socket_fd_ = -1;
  ClientCallback callback_ = [] (char const*, int) -> void {};
};

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_CLIENT_H_

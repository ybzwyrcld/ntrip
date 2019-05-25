#ifndef NTRIPLIB_CATSER_NTRIP_CASTER_H_
#define NTRIPLIB_CATSER_NTRIP_CASTER_H_

#include <string>
#include <list>
#include <vector>

#include "util/ntrip_mountpoint.h"


#define MAX_LEN 65536

class NtripCaster {
 public:
  NtripCaster() = default;
  ~NtripCaster();
  bool Init(const int &port, const int &sock_count);
  bool Init(const char *ip, const int &port, const int &sock_count);
  int NtripCasterWait(const int &time_out);
  int AcceptNewClient(void);
  int RecvData(const int &sock, char *recv_buf);
  int SendData(const int &sock, const char *send_buf, const int &buf_len);
  void Run(const int &time_out);
  int ParseData(const int &sock, char *recv_buf, const int &len);

 private:
  int listen_sock_ = -1;
  int epoll_fd_ = -1;
  int max_count_ = 0;
  struct epoll_event *epoll_events_ = nullptr;
  std::list<MountPoint> mount_point_list_;
  std::vector<std::string> ntrip_str_list_;
};

#endif // NTRIPLIB_CATSER_NTRIP_CASTER_H_

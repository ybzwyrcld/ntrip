#ifndef NTRIPLIB_UTIL_NTRIP_MOUNTPOINT_H_
#define NTRIPLIB_UTIL_NTRIP_MOUNTPOINT_H_

#include <list>


struct MountPoint {
  int server_fd;
  char username[16];
  char password[16];
  char mount_point_name[16];
  std::list<int> conn_sock;
};

#endif // NTRIPLIB_UTIL_NTRIP_MOUNTPOINT_H_

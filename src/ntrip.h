#ifndef NTRIPLIB_NTRIP_H_
#define NTRIPLIB_NTRIP_H_


#include <string>
#include <thread>
#include <list>
#include <vector>

namespace libntrip {

class Ntrip {
 public:
   Ntrip() = default;
   ~Ntrip();

   void InitClient(const std::string &ip, const int &port,
                   const std::string &user, const std::string &passwd,
                   const std::string &mountpoint) {
       client_server_ip_ = ip;
       client_server_port_ = port;
       client_server_user_ = user;
       client_server_passwd_ = passwd;
       client_server_mountpoint_ = mountpoint;
   }

   bool ClientStart(void);
   void ClientStop(void);
   bool ClientBufferEmpty(void) {
     return client_buffer_list_.empty();
   }
   bool ClientBuffer(std::vector<char> *buffer) {
     if (ClientBufferEmpty() || buffer == nullptr) return false;
     buffer->clear();
     buffer->assign(client_buffer_list_.front().begin(),
                    client_buffer_list_.front().end());
     client_buffer_list_.pop_front();
     return true;
   }

 private:
  // Thread handler.
  void ClientTheradHandler(void);

  bool client_thread_is_running_ = false;
  std::thread client_thread_;
  std::string client_server_ip_;
  int client_server_port_;
  std::string client_server_mountpoint_;
  std::string client_server_user_;
  std::string client_server_passwd_;
  int client_socket_ = -1;
  std::list<std::vector<char>> client_buffer_list_;
};

}

#endif  // NTRIPLIB_NTRIP_H_

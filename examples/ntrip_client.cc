#include "ntrip.h"

#include <unistd.h>

#include <string>
#include <vector>


using libntrip::Ntrip;

int main(void) {
  std::string ip = "127.0.0.1";
  int port = 8192;
  std::string user = "user";
  std::string passwd  = "passwd";
  std::string mountpoint  = "mountpoint";

  Ntrip ntrip_client;
  ntrip_client.InitClient(ip, port, user, passwd, mountpoint);
  ntrip_client.ClientStart();

  int cnt = 5;
  std::vector<char> msg(1024);
  while (cnt--) {
    if (!ntrip_client.ClientBufferEmpty()) {
      msg.clear();
      if (ntrip_client.ClientBuffer(&msg)) {
        for (auto ch : msg) {
          printf("%02X ", ch);
        }
        printf("\n");
      }
    }
    sleep(1);
  }
  ntrip_client.ClientStop();
  return 0;
}

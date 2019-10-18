#include "client.h"

#include <unistd.h>

#include <string>
#include <vector>


using libntrip::Client;

int main(void) {
  std::string ip = "127.0.0.1";
  int port = 8192;
  std::string user = "user";
  std::string passwd  = "passwd";
  std::string mountpoint  = "mountpoint";

  Client ntrip_client;
  ntrip_client.Init(ip, port, user, passwd, mountpoint);
  ntrip_client.Start();

  int cnt = 5;
  std::vector<char> msg(1024);
  while (cnt--) {
    if (!ntrip_client.BufferEmpty()) {
      msg.clear();
      if (ntrip_client.Buffer(&msg)) {
        for (auto ch : msg) {
          printf("%02X ", ch);
        }
        printf("\n");
      }
    }
    sleep(1);
  }
  ntrip_client.Stop();
  return 0;
}

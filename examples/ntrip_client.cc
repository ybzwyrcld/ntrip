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

#include "ntrip_client.h"

#include <unistd.h>
#include <stdint.h>

#include <string>
#include <vector>


using libntrip::NtripClient;

int main(void) {
  std::string ip = "127.0.0.1";
  int port = 8090;
  std::string user = "test01";
  std::string passwd  = "123456";
  std::string mountpoint  = "RTCM32";

  NtripClient ntrip_client;
  ntrip_client.Init(ip, port, user, passwd, mountpoint);
  ntrip_client.Run();

  int cnt = 5;
  std::vector<char> msg(1024);
  while (cnt--) {
    if (!ntrip_client.BufferEmpty()) {
      msg.clear();
      if (ntrip_client.Buffer(&msg)) {
        for (auto ch : msg) {
          printf("%02X ", static_cast<uint8_t>(ch));
        }
        printf("\n");
      }
    }
    sleep(1);
  }
  ntrip_client.Stop();
  return 0;
}

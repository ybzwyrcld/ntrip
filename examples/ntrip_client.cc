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

#include "ntrip_util.h"


using libntrip::NtripClient;

int main(void) {
  std::string ip = "127.0.0.1";
  int port = 8090;
  std::string user = "test01";
  std::string passwd = "123456";
  std::string mountpoint = "RTCM32";

  NtripClient ntrip_client;
  ntrip_client.Init(ip, port, user, passwd, mountpoint);
  ntrip_client.OnReceived([](const char *buffer, const int &size) {
    printf("Recv[%d]: ", size);
    for (int i = 0; i < size; ++i) {
      printf("%02X ", static_cast<uint8_t>(buffer[i]));
    }
    printf("\n");
  });
  std::string gga;
  if (libntrip::GetGGAFrameData(22.57311, 113.94905, 10.0, &gga) == 0) {
    printf("GGA buffer: %s\n", gga.c_str());
    ntrip_client.set_gga_buffer(gga);
  }
  ntrip_client.Run();
  // Exit the program after 10 seconds.
  int cnt = 10;
  while (cnt--) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  ntrip_client.Stop();
  return 0;
}


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

#include "ntrip/ntrip_server.h"

#include <unistd.h>
#include <stdint.h>

#include <string>
#include <vector>


namespace {

// RTCM3.2 format example data.
constexpr uint8_t kExmapleData[] = {
  0xD3, 0x00, 0x40, 0x41, 0x2E, 0x06, 0x44, 0x19, 0x1E, 0xF5, 0x00,
  0xA4, 0x00, 0x00, 0x10, 0xB6, 0x11, 0x08, 0xC2, 0xE8, 0x1D, 0x58,
  0x1A, 0x72, 0xC8, 0x46, 0xCD, 0x1A, 0x08, 0xEA, 0x81, 0x2C, 0x3E,
  0xDC, 0x1B, 0xBB, 0xD9, 0x5D, 0x90, 0x61, 0xE8, 0x05, 0x2F, 0xFB,
  0x89, 0x9A, 0x4D, 0xCC, 0xEB, 0xFE, 0x4C, 0x25, 0x28, 0xFB, 0x6C,
  0xDA, 0x7F, 0x61, 0x8E, 0x60, 0x9C, 0xBF, 0xFB, 0x6A, 0x2D, 0x30,
  0x02, 0x19, 0x8F, 0x73
};

constexpr int kExmapleDataLength = sizeof(kExmapleData);

using libntrip::NtripServer;

} // namespace

int main(void) {
  std::string ip = "127.0.0.1";
  int port = 8090;
  std::string user = "test01";
  std::string passwd  = "123456";
  // Mount point must be consistent with 'ntrip_str',
  // 'STR;{mountpoint};{mountpoint};'.
  std::string mountpoint  = "RTCM32";
  std::string ntrip_str = "STR;RTCM32;RTCM32;RTCM 3.2;"
                          "1004(1),1005/1007(5),PBS(10);2;GPS;SGNET;CHN;"
                          "31;121;1;1;SGCAN;None;B;N;0;;";

  NtripServer ntrip_server;
  ntrip_server.Init(ip, port, user, passwd, mountpoint, ntrip_str);
  if (ntrip_server.Run()) {
     while (ntrip_server.service_is_running()) {
      if (ntrip_server.SendData((char*)kExmapleData, kExmapleDataLength) != 0) {
        break;
      }
      printf("Send example data success!!!\n");
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }
  ntrip_server.Stop();
  return 0;
}

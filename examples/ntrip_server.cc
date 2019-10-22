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

#include "ntrip_server.h"

#include <unistd.h>
#include <stdint.h>

#include <string>
#include <vector>


using libntrip::NtripServer;

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

  if (!ntrip_server.Run()) exit(1);

  while (1) {
    // TODO(mengyuming@hotmail.com) : Add your code in here.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  ntrip_server.Stop();
  return 0;
}

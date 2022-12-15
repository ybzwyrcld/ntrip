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

#include <stdint.h>

#include <string>
#include <vector>

#include "ntrip/ntrip_client.h"
#include "ntrip/ntrip_util.h"


using libntrip::NtripClient;

int main(void) {
  std::string ip = "127.0.0.1";
  int port = 8090;
  std::string user = "test01";
  std::string passwd = "123456";
  std::string mountpoint = "RTCM32";

  NtripClient ntrip_client;
  ntrip_client.Init(ip, port, user, passwd, mountpoint);
  ntrip_client.OnReceived([] (const char *buffer, int size) {
    printf("Recv[%d]: ", size);
    for (int i = 0; i < size; ++i) {
      printf("%02X ", static_cast<uint8_t>(buffer[i]));
    }
    printf("\n");
  });
  // std::string gga;
  // if (libntrip::GetGGAFrameData(22.57311, 113.94905, 10.0, &gga) == 0) {
  //   printf("GGA buffer: %s\n", gga.c_str());
  //   ntrip_client.set_gga_buffer(gga);
  // }
  ntrip_client.set_location(22.57311, 113.94905);
  ntrip_client.set_report_interval(1);
  ntrip_client.Run();
  std::this_thread::sleep_for(std::chrono::seconds(1));  // Maybe take longer?
  while (ntrip_client.service_is_running()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  ntrip_client.Stop();
  return 0;
}

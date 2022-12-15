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

// @File    :  ntrip_client_to_ntrip_server_exam.cc
// @Version :  1.0
// @Time    :  2021/07/21 09:16:15
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <stdint.h>

#include <string>
#include <vector>

#include "ntrip/ntrip_client.h"
#include "ntrip/ntrip_server.h"
#include "ntrip/ntrip_util.h"


namespace {

using libntrip::NtripClient;
using libntrip::NtripServer;


constexpr char kNtripString[] =
    "STR;RTCM32;RTCM32;RTCM 3.2;1004(1),1005/1007(5),PBS(10);"
    "2;GPS;SGNET;CHN;31;121;1;1;SGCAN;None;B;N;0;;";

}  // namespace

int main(void) {
  NtripServer ntrip_server;
  ntrip_server.Init("127.0.0.1", 8090, "test01", "123456",
      "RTCM32", kNtripString);
  NtripClient ntrip_client;
  ntrip_client.Init("127.0.0.1", 8090, "test01", "123456", "RTCM32");
  ntrip_client.OnReceived([&] (const char *buffer, const int &size) {
    printf("Recv[%d]: ", size);
    for (int i = 0; i < size; ++i) {
      printf("%02X ", static_cast<uint8_t>(buffer[i]));
    }
    printf("\n");
    // 转发到NtripServer.
    if (ntrip_server.service_is_running()) {
      ntrip_server.SendData(buffer, size);
    }
  });
  ntrip_client.set_location(22.57311, 113.94905);
  ntrip_client.set_report_interval(1);
  ntrip_client.Run();
  do {
    if (!ntrip_client.service_is_running()) {
      ntrip_client.Run();
    }
    if (!ntrip_server.service_is_running()) {
      ntrip_server.Run();
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
  } while (1);
  ntrip_server.Stop();
  ntrip_client.Stop();
  return 0;
}

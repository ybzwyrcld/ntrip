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

#include "ntrip/ntrip_caster.h"


using libntrip::NtripCaster;

int main(int argc, char *argv[]) {
  NtripCaster ntrip_caster;
  ntrip_caster.Init(8090, 30, 2000);
  // ntrip_caster.Init("127.0.0.1", 8090, 10, 2000);
  ntrip_caster.Run();
  std::this_thread::sleep_for(std::chrono::seconds(1));  // Maybe take longer?
  while (ntrip_caster.service_is_running()) {
    // TODO(mengyuming@hotmail.com) : Add your code in here.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  ntrip_caster.Stop();
  return 0;
}

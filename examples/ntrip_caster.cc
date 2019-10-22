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

#include "ntrip_caster.h"


using libntrip::NtripCaster;

int main(int argc, char *argv[]) {
  NtripCaster my_caster;
  my_caster.Init(8090, 30, 2000);
  // my_caster.Init("127.0.0.1", 8090, 10, 2000);
  my_caster.Run();

  while (1) {
    // TODO(mengyuming@hotmail.com) : Add your code in here.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return 0;
}


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

#include "util.h"

#include <string.h>


namespace libntrip {

// GPGGA format example.
constexpr char gpgga_buffer[] =
    "$GPGGA,083552.00,3000.0000000,N,11900.0000000,E,"
    "1,08,1.0,0.000,M,100.000,M,,*57\r\n";

//
// Ntrip util.
//

constexpr char kBase64CodingTable[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int GetIndexByChar(const char &ch) {
  for (int i = 0; i < 64; ++i) {
    if (ch == kBase64CodingTable[i]) {
      return i;
    }
  }
  return -1;
}

inline char GetCharByIndex(const int &index) {
  return kBase64CodingTable[index];
}

int Base64Encode(const char *src, char *result) {
  char temp[3] = {0};
  int i = 0, j = 0, count = 0;
  int len = strlen(src);
  if (len == 0) {
    return -1;
  }

  if (len%3 != 0) {
    count = 3 - len%3;
  }

  while (i < len) {
    strncpy(temp, src+i, 3);
    result[j+0] = GetCharByIndex((temp[0]&0xFC) >> 2);
    result[j+1] = GetCharByIndex(((temp[0]&0x3) << 4) | ((temp[1]&0xF0) >> 4));
    if (temp[1] == 0) {
      break;
    }
    result[j+2] = GetCharByIndex(((temp[1]&0xF) << 2) | ((temp[2]&0xC0) >> 6));
    if (temp[2] == 0) {
      break;
    }
    result[j+3] = GetCharByIndex(temp[2]&0x3F);
    i += 3;
    j += 4;
    memset(temp, 0x0, 3);
  }

  while (count) {
    result[j+4-count] = '=';
    --count;
  }

  return 0;
}

}  // namespace libntrip

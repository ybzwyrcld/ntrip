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

#include "ntrip_util.h"

#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <string>
#include <fstream>


namespace libntrip {

namespace {

double DegreeConvertToDDMM(double const& degree) {
  int deg = static_cast<int>(floor(degree));
  double minute = degree - deg*1.0;
  return (deg*1.0 + minute*60.0/100.0);
}

}  // namespace

//
// Ntrip util.
//

constexpr char kBase64CodingTable[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void PrintCharArray(const char *src, const int &len) {
  for (int i = 0; i < len; ++i) {
      printf("%c", (unsigned char)src[i]);
        }
    printf("\n");
}

void PrintCharArrayHex(const char *src, const int &len) {
  for (int i = 0; i < len; ++i) {
      printf("%02x ", (unsigned char)src[i]);
        }
    printf("\n");
}

int BccCheckSumCompareForGGA(const char *src) {
  int sum = 0;
  int num = 0;

  sscanf(src, "%*[^*]*%x", &num);
  for (int i = 1; src[i] != '*'; ++i) {
    sum ^= src[i];
  }
  return sum - num;
}

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

int Base64Decode(const char *src, char *user, char *passwd) {
  char result[64] = {0};
  char temp[4] = {0};
  int i = 0;
  int j = 0;
  int len = strlen(src);
  if ((len == 0) || (len%4 != 0)) {
    return -1;
  }

  while (i < len) {
    strncpy(temp, src+i, 4);
    result[j+0] = ((GetIndexByChar(temp[0])&0x3F) << 2) |
                  ((GetIndexByChar(temp[1])&0x3F) >> 4);
    if (temp[2] == '=') {
      break;
    }
    result[j+1] = ((GetIndexByChar(temp[1])&0xF) << 4) |
                  ((GetIndexByChar(temp[2])&0x3F) >> 2);
    if (temp[3] == '=') {
      break;
    }
    result[j+2] = ((GetIndexByChar(temp[2])&0x3) << 6) |
                  ((GetIndexByChar(temp[3])&0x3F));
    i += 4;
    j += 3;
    memset(temp, 0x0, 4);
  }
  sscanf(result, "%[^:]%*c%[^\n]", user, passwd);

  return 0;
}


int GetSourcetable(const char *path, char *data, const int &data_len) {
  if (access(path, F_OK) == -1) return -1;

  std::ifstream ifs;
  ifs.open(path, std::ios::in | std::ios::binary);
  if (ifs.is_open()) {
    ifs.seekg(0, std::ios::end);
    int length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    char *data = new char[length];
    ifs.read(data, length);
    ifs.close();

    snprintf(data, data_len,
             "SOURCETABLE 200 OK\r\n"
             "Server: %s\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %d\r\n"
             "%s"
             "ENDSOURCETABLE\r\n",
             kCasterAgent, length, data);
    delete [] data;
  }

  return 0;
}

int GetGGAFrameData(double const& latitude,
                    double const& longitude,
                    double const& altitude,
                    std::string* const gga_str) {
  if (gga_str == nullptr) return -1;
  char src[256] = {0};
  time_t t = time(nullptr);
  struct tm *tt = localtime(&t);
  double timestamp[3];
  timestamp[0] = tt->tm_hour >= 8 ? tt->tm_hour - 8 : tt->tm_hour + 24 - 8;
  timestamp[1] = tt->tm_min;
  timestamp[2] = tt->tm_sec;
  char *ptr = src;
  ptr += snprintf(ptr, sizeof(src)+src-ptr,
      "$GPGGA,%02.0f%02.0f%05.2f,%012.7f,%s,%013.7f,%s,1,"
      "10,1.2,%.4f,M,-2.860,M,,0000",
      timestamp[0], timestamp[1], timestamp[2],
      fabs(DegreeConvertToDDMM(latitude))*100.0,
      latitude > 0.0 ? "N" : "S",
      fabs(DegreeConvertToDDMM(longitude))*100.0,
      longitude > 0.0 ? "E" : "W",
      altitude);
  uint8_t checksum = 0;
  for (char *q = src + 1; q <= ptr; q++) {
    checksum ^= *q; // check sum.
  }
  ptr += snprintf(ptr, sizeof(src)+src-ptr, "*%02X%c%c", checksum, 0x0D, 0x0A);
  *gga_str = std::string(src, ptr-src);
  return BccCheckSumCompareForGGA(gga_str->c_str());
}

}  // namespace libntrip

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

#ifndef NTRIPLIB_NTRIP_UTIL_H_
#define NTRIPLIB_NTRIP_UTIL_H_

#include <string>


namespace libntrip {

constexpr char kCasterAgent[] = "NTRIP NTRIPCaster/20191018";
constexpr char kClientAgent[] = "NTRIP NTRIPClient/20191018";
constexpr char kServerAgent[] = "NTRIP NTRIPServer/20191018";

void PrintCharArray(const char *src, const int &len);
void PrintCharArrayHex(const char *src, const int &len);
int BccCheckSumCompareForGGA(const char *src);
int Base64Encode(const char *src, char *result);
int Base64Decode(const char *src, char *user, char *passwd);
int GetSourcetable(const char *path, char *data, const int &data_len);
int GetGGAFrameData(double const& latitude, double const& longitude,
                    double const& altitude, std::string* const gga_str);

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_UTIL_H_

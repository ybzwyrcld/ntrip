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

#ifndef NTRIPLIB_NTRIP_UTIL_H_
#define NTRIPLIB_NTRIP_UTIL_H_

#include <string>


namespace libntrip {

constexpr char kCasterAgent[] = "NTRIP NTRIPCaster/20210620";
constexpr char kClientAgent[] = "NTRIP NTRIPClient/20210620";
constexpr char kServerAgent[] = "NTRIP NTRIPServer/20210620";

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

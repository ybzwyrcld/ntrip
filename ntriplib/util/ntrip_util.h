#ifndef NTRIPLIB_UTIL_NTRIP_UTIL_H_
#define NTRIPLIB_UTIL_NTRIP_UTIL_H_

const char kCasterAgent[] = "NTRIP NTRIPCaster/20190525";
const char kClientAgent[] = "NTRIP NTRIPClient/20190525";
const char kServerAgent[] = "NTRIP NTRIPServer/20190525";
const char kBase64CodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz0123456789+/";

void PrintChar(const char *src, const int &len);
void PrintCharHex(const char *src, const int &len);
int BccCheckSumCompareForGGA(const char *src);
int GetIndexByChar(const char &ch);
char GetCharByIndex(const int &index);
int Base64Encode(const char *src, char *result);
int Base64Decode(const char *src, char *user, char *passwd);
int GetSourcetable(char *data, const int &data_len);

#endif // NTRIPLIB_UTIL_NTRIP_UTIL_H_

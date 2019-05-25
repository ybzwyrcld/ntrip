#include "util/ntrip_util.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <string>


void PrintChar(const char *src, const int &len) {
  for (int i = 0; i < len; ++i) {
    printf("%c", (unsigned char)src[i]);
  }
  printf("\n");
}

void PrintCharHex(const char *src, const int &len) {
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
    result[j+0] = GetCharByIndex((temp[0]&0xFC)>>2);
    result[j+1] = GetCharByIndex(((temp[0]&0x3)<<4) | ((temp[1]&0xF0)>>4));
    if (temp[1] == 0) {
      break;
    }
    result[j+2] = GetCharByIndex(((temp[1]&0xF)<<2) | ((temp[2]&0xC0)>>6));
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
  if (len==0 || len%4!=0) {
    return -1;
  }

  while (i < len) {
    strncpy(temp, src+i, 4);
    result[j+0] = ((GetIndexByChar(temp[0])&0x3F)<<2) |
                  ((GetIndexByChar(temp[1])&0x3F)>>4);
    if (temp[2] == '=') {
      break;
    }
    result[j+1] = ((GetIndexByChar(temp[1])&0xF)<<4) |
                  ((GetIndexByChar(temp[2])&0x3F)>>2);
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


int GetSourcetable(char *data, const int &data_len) {
  std::ifstream ifs;
  ifs.open("./config/sourcetable.dat", std::ios::in | std::ios::binary);
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


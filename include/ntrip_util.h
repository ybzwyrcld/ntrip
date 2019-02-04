#ifndef __NTRIP_UTIL_H__
#define __NTRIP_UTIL_H__

const char caster_agent[] = "NTRIP NTRIPCaster/20190204";
const char client_agent[] = "NTRIP NTRIPClient/20190204";
const char server_agent[] = "NTRIP NTRIPServer/20190204";
const char base64_code_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void print_char(const char *src, const int &len);
void print_char_hex(const char *src, const int &len);
int check_sum(const char *src);
int ch2index(const char &ch);
char index2chr(const int &index);
int base64_encode(const char *src, char *result);
int base64_decode(const char *src, char *usr, char *pwd);
int get_sourcetable(char *data, const int &data_len);

#endif //__NTRIP_UTIL_H__

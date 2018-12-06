#ifndef __NTRIP_UTIL_H__
#define __NTRIP_UTIL_H__

const char caster_agent[] = "NTRIP NTRIPCaster/20181206";
const char client_agent[] = "NTRIP NTRIPClient/20181206";
const char server_agent[] = "NTRIP NTRIPServer/20181206";
const char base64_code_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void print_char(char *src, int len);
void print_char_hex(char *src, int len);
int check_sum(char *src);
int ch2index(char ch);
char index2chr(int index);
int base64_encode(char *src, char *result);
int base64_decode(char *src, char *usr, char *pwd);
int get_sourcetable(char *data, int data_len);

#endif //__NTRIP_UTIL_H__

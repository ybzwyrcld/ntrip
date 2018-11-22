#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream> 
#include <cstdio> 
#include <cstdlib> 
#include <cstring> 

using std::cin;
using std::cout;
using std::endl;

const char caster_agent[] = "NTRIP TheXiiNTRIPCaster/20180926";
const char client_agent[] = "NTRIP TheXiiNTRIPClient/20180926";
const char server_agent[] = "NTRIP TheXiiNTRIPServer/20180926";
const char base64_code_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int print_char(char *src, int len);
int check_sum(char *src);
int ch2index(char ch);
char index2chr(int index);
int base64_encode(char *src, char *result);
int base64_decode(char *src, char *usr, char *pwd);
int get_sourcetable(char *data, int data_len);

#endif // __UTIL_H__

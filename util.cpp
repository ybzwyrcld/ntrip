#include <unistd.h>
#include "util.h"

int print_char(char *src, int len)
{
	for(int i = 0; i < len; ++i){
		cout << src[i];
	}
}

int check_sum(char *src)
{
	char *ch = new char[3];
	int sum = 0, num = 0;

	sscanf(src, "%*[^*]*%x", &num);
	for(int i = 1; src[i] != '*'; ++i){
		sum ^= src[i];
	}
	return sum - num;
}

int ch2index(char ch)
{
	int i;
	for(i=0; i< strlen(base64_code_table); ++i){
		if(ch == base64_code_table[i])
			return i;
	}
	return -1;
}

char index2chr(int index)
{
	return base64_code_table[index];
}


int base64_encode(char *src, char *result)
{
	char temp[3] = {0};
	int i = 0, j = 0, count = 0;
	int len = strlen(src);
	if(len==0)
		return -1;

	if(len%3 != 0){
		count = 3 - len%3;
	}

	while(i < len){
		strncpy(temp, src+i, 3);
		result[j+0] = index2chr((temp[0]&0xFC)>>2);
		result[j+1] = index2chr(((temp[0]&0x3)<<4) | ((temp[1]&0xF0)>>4));
		if(temp[1] == 0)
			break;
		result[j+2] = index2chr(((temp[1]&0xF)<<2) | ((temp[2]&0xC0)>>6));
		if(temp[2] == 0)
			break;
		result[j+3] = index2chr(temp[2]&0x3F);
		i+=3;
		j+=4;
		memset(temp, 0x0, 3);
	}

	while(count){
		result[j+4-count] = '=';
		--count;
	}

	return 0;
}


int base64_decode(char *src, char *usr, char *pwd)
{
	char result[64] = {0};
	char temp[4] = {0};
	int i = 0;
	int j = 0;
	int len = strlen(src);
	if(len==0 || len%4!=0)
		return -1;

	while(i < len){
		strncpy(temp, src+i, 4);
		result[j+0] = ((ch2index(temp[0])&0x3F) << 2) | ((ch2index(temp[1])&0x3F) >> 4);
		if(temp[2] == '=')
			break;
		result[j+1] = ((ch2index(temp[1])&0xF) << 4) | ((ch2index(temp[2])&0x3F) >> 2);
		if(temp[3] == '=')
			break;
		result[j+2] = ((ch2index(temp[2])&0x3) << 6) | ((ch2index(temp[3])&0x3F));
		i+=4;
		j+=3;
		memset(temp, 0x0, 4);
	}
	sscanf(result, "%[^:]%*c%[^\n]", usr, pwd);

	return 0;
}


int get_sourcetable(char *data, int data_len)
{
	int fsize = 0;

	FILE *fp = fopen("./sourcetable.dat", "r");
	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		fsize = (int)ftell(fp);
		rewind(fp);
	}

	char *source = (char *)malloc(fsize);
	fread(source, sizeof(char), fsize, fp);

	snprintf(data, data_len, "SOURCETABLE 200 OK\r\n"
		"Server: %s\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: %d\r\n"
		"%s"
		"ENDSOURCETABLE\r\n",
		caster_agent, fsize , source);

	fclose(fp);
	free(source);
	source = NULL;

	return 0;
}


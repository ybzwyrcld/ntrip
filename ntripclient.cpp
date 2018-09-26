#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include "util.h"

int main(int argc, char *argv[])
{
	int m_sock;
	time_t start, stop;
	char recv_buf[1024] = {0};

	struct sockaddr_in source_addr;
	memset(&source_addr, 0, sizeof(&source_addr));
	source_addr.sin_family = AF_INET;
	source_addr.sin_port = htons(12345);
	source_addr.sin_addr.s_addr = inet_addr("192.168.2.246");

	char request[] = {
		"GET /RTCM32 HTTP/1.1\r\n"
		"User-Agent: NTRIP TheXiiNTRIPCaster/20180926\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"Authorization: Basic dGVzdDAxOnRlc3Rpbmc=\r\n" 
	};

	char gpgga[] = "$GPGGA,083552.00,3000.0000000,N,11900.0000000,E,1,08,1.0,0.000,M,100.000,M,,*57\r\n";
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sock == -1) {
		printf("socket fail\n");
		exit(1);
	}

	int ret = connect(m_sock, (struct sockaddr *)&source_addr, sizeof(source_addr));
	if(ret < 0){
		printf("connect fail\n");
		exit(1);
	}

	ret = send(m_sock, request, strlen(request), 0);
	if(ret < 0){
		printf("send fail\n");
		exit(1);
	}

	start = time(NULL);
	while(1){
		ret = recv(m_sock, (void *)recv_buf, sizeof(recv_buf), 0);
		if(ret > 0){
			stop = time(NULL);
			if(!strncmp(recv_buf, "ICY 200 OK\r\n", 12)){
				printf("request connect ok\n");
#if 1
				ret = send(m_sock, gpgga, strlen(gpgga), 0);
				if(ret < 0){
					printf("send fail\n");
				exit(1);
				}
				printf("send gpgga ok\n");
#endif
			}
			printf("recv data:[%d] use time:[%d]\n", ret, (int)(stop - start));
			print_char(recv_buf, ret);
			start = time(NULL);
		}
	}

	close(m_sock);

	return  0;
}


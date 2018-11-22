#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ntrip_util.h>

int main(int argc, char *argv[])
{
	int m_sock;
	time_t start, stop;
	char recv_buf[1024] = {0};
	char request[1024] = {0};
	char userinfo_raw[48] = {0};
  	char userinfo[64] = {0};
 
	char server_ip[] = "192.168.1.118";
	int server_port = 12345;
   	
	char mountpoint[] = "RTCM32";
	char user[] = "test01";
	char passwd[] = "testing";
	char gpgga[] = "$GPGGA,083552.00,3000.0000000,N,11900.0000000,E,1,08,1.0,0.000,M,100.000,M,,*57\r\n";

	struct sockaddr_in source_addr;
	memset(&source_addr, 0, sizeof(&source_addr));
	source_addr.sin_family = AF_INET;
	source_addr.sin_port = htons(server_port);
	source_addr.sin_addr.s_addr = inet_addr(server_ip);
	
	/* Generate base64 encoding of user and passwd. */
	snprintf(userinfo_raw, 63 , "%s:%s", user, passwd);
	base64_encode(userinfo_raw, userinfo);
 
    /* Generate request data format of ntrip. */
	snprintf(request, 1023,
		"GET /%s HTTP/1.1\r\n"
		"User-Agent: %s\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"Authorization: Basic %s\r\n" 
		, mountpoint, client_agent, userinfo);

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sock == -1) {
		printf("create socket fail\n");
		exit(1);
	}

	int ret = connect(m_sock, (struct sockaddr *)&source_addr, sizeof(source_addr));
	if(ret < 0){
		printf("connect caster fail\n");
		exit(1);
	}

	ret = send(m_sock, request, strlen(request), 0);
	if(ret < 0){
		printf("send request fail\n");
		exit(1);
	}

	/* Wait for request to connect caster success. */
	while(1){
		ret = recv(m_sock, (void *)recv_buf, sizeof(recv_buf), 0);
		if(ret > 0 && !strncmp(recv_buf, "ICY 200 OK\r\n", 12)){
			ret = send(m_sock, gpgga, strlen(gpgga), 0);
			if(ret < 0){
				printf("send gpgga data fail\n");
				exit(1);
			}
			printf("send gpgga data ok\n");
			break;
		}
	}

	/* Receive data returned by caster. */
	while(1){
		start = time(NULL);
		ret = recv(m_sock, (void *)recv_buf, sizeof(recv_buf), 0);
		stop = time(NULL);
		if(ret > 0){
			printf("recv data:[%d] used time:[%d]\n", ret, (int)(stop - start));
			print_char(recv_buf, ret);
		}
	}

	close(m_sock);

	return  0;
}


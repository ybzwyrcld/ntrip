#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include "util.h"

int main(void)
{
	int m_sock;
	int ret;
	char recv_buf[1024] = {0};	

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(&server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(12345);
	server_addr.sin_addr.s_addr = inet_addr("192.168.2.246");

	char send_buf[1024] = {
		"POST /RTCM32 HTTP/1.1\r\n"
		"Host: 192.168.2.246:12345\r\n"
		"Ntrip-Version: Ntrip/2.0\r\n"
		"User-Agent: NTRIP TheXiiNTRIPServer/20180926\r\n"
		"Authorization: Basic dGVzdDAxOnRlc3Rpbmc=\r\n"
		"Ntrip-STR: \r\n"
		"Connection: close\r\n"
		"Transfer-Encoding: chunked\r\n"
	};
	
	char data[16] = {0xd3, 0x00, 0x70, 0x8e, 0x43, 0x56, 0x45, 0x00, 0x00, 0x55, 0xfb, 0x89, 0xff, 0xff, '\r', '\n'};

	m_sock= socket(AF_INET, SOCK_STREAM, 0);
	if(m_sock == -1) {
		exit(1);
	}

	ret = connect(m_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret < 0){
		exit(1);
	}

	ret = send(m_sock, send_buf, strlen(send_buf), 0);	
	if(ret < 0){
		exit(1);
	}

	cout << "send source ok" << endl;
	ret = recv(m_sock, recv_buf, 1024, 0);
	if(ret > 0 && !strncmp(recv_buf, "ICY 200 OK\r\n", 12)){
		printf("recv: \n%s", recv_buf);
		while(1){
			memset(recv_buf, 0x0, 1024);
			ret = send(m_sock, data, sizeof(data), 0);
			if(ret > 0)
				cout << "send data ok" << endl;
			sleep(1);
		}
	}

	memset(recv_buf, 0x0, 1024);

	close(m_sock);

	return 0;	
}


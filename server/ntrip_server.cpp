#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ntrip_util.h>

int main(void)
{
	int m_sock;
	int ret;
	char recv_buf[1024] = {0};	
	char send_buf[1024] = {0};
	char userinfo_raw[48] = {0};
	char userinfo[64] = {0};

	char server_ip[] = "192.168.1.118";
	int server_port = 12345;
	char mountpoint[] = "RTCM32";
	char user[] = "test01";
	char passwd[] = "testing";

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(&server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	/* Generate base64 encoding of user and passwd. */
	snprintf(userinfo_raw, 63 , "%s:%s", user, passwd); 
	base64_encode(userinfo_raw, userinfo);

	/* Generate request example_data format of ntrip. */
	snprintf(send_buf, 1023 , 
		"POST /%s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"Ntrip-Version: Ntrip/2.0\r\n"
		"User-Agent: %s\r\n"
		"Authorization: Basic %s\r\n"
		"Ntrip-STR: \r\n"
		"Connection: close\r\n"
		"Transfer-Encoding: chunked\r\n"
		, mountpoint, 
		server_ip, server_port, 
		server_agent, 
		userinfo);
	
	unsigned char example_data[] = {
			0xd3, 0x00, 0x70, 0x8e, 0x43, 0x56, 0x45, 0x00, 0x00, 0x55, 0xfb, 0x89, 0xff, 0xff, '\r', '\n'};

	m_sock= socket(AF_INET, SOCK_STREAM, 0);
	if(m_sock == -1) {
		exit(1);
	}

	ret = connect(m_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret < 0){
		printf("connect to caster fail!!!");
		exit(1);
	}

	ret = send(m_sock, send_buf, strlen(send_buf), 0);	
	if(ret < 0){
		printf("send request fail!!!");
		exit(1);
	}

	/* Wait for request to connect caster success. */
	while(1){
		memset(recv_buf, 0x0, 1024);
		ret = recv(m_sock, recv_buf, 1024, 0);
		printf("recv: \n%s\n", recv_buf);
		if(ret > 0 && !strncmp(recv_buf, "ICY 200 OK\r\n", 12)){
			break;
		}
	}

	/* Send data to caster. */
	while(1){
		ret = send(m_sock, example_data, sizeof(example_data), 0);
		if(ret > 0){
			printf("send example_data ok\n");
		}
		sleep(1);
	}

	close(m_sock);

	return 0;	
}


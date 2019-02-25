#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include <iostream>
#include <fstream>

#include <ntrip_util.h>

using std::ifstream;
using std::ofstream;
using std::ios;

int main(void)
{
	int m_sock;
	int ret;
	ofstream ofile;
	char recv_buf[1024] = {0};	
	char send_buf[1024] = {0};
	char userinfo_raw[48] = {0};
	char userinfo[64] = {0};

	char server_ip[] = "127.0.0.1";
	int server_port = 8090;
	char mountpoint[] = "RTCM32";
	char user[] = "test01";
	char passwd[] = "testing";

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
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
		"Ntrip-STR: STR;%s;%s;RTCM 3.2;1004(1),1005/1007(5),PBS(10);2;GPS;SGNET;CHN;31;121;1;1;SGCAN;None;B;N;0;;\r\n"
		"Connection: close\r\n"
		"Transfer-Encoding: chunked\r\n"
		, mountpoint, 
		server_ip, server_port, 
		server_agent,
		userinfo,
		mountpoint, mountpoint
	);
	
	unsigned char example_data[] = {
			0xd3, 0x00, 0x70, 0x8e, 0x43, 0x56, 0x45, 0x00, 0x00, 0x55, 0xfb, 0x89, 0xff, 0xff, '\r', '\n'};

	m_sock= socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == -1) {
		exit(1);
	}

	/* Connect to caster. */
	ret = connect(m_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		printf("connect to caster fail!!!\n");
		exit(1);
	}

	/* Set socket no block. */
	int flags = fcntl(m_sock, F_GETFL);
	fcntl(m_sock, F_SETFL, flags | O_NONBLOCK);

	/* Send request data. */
	ret = send(m_sock, send_buf, strlen(send_buf), 0);	
	if (ret < 0) {
		printf("send request fail!!!\n");
		exit(1);
	}

	/* Wait for request to connect caster success. */
	printf("waitting...\n");
	while (1) {
		memset(recv_buf, 0x0, 1024);
		ret = recv(m_sock, recv_buf, 1024, 0);
		if (ret > 0) {
			printf("recv: \n%s", recv_buf);
			if (!strncmp(recv_buf, "ICY 200 OK\r\n", 12))
				break;
		}
	}

	/* Waiting for a GGA data, optional. */
	while (1) {
		memset(recv_buf, 0x0, 1024);
		ret = recv(m_sock, recv_buf, 1024, 0);
		if (ret > 0) {
			if (!strncmp(recv_buf, "$GNGGA", 6)  || !strncmp(recv_buf, "$GPGGA", 6)) {
				send(m_sock, "recv ok", 8, 0);
				break;
			}
		}
	}

	/* Send data to caster. */
	while (1) {
		ret = send(m_sock, example_data, sizeof(example_data), 0);
		if (ret > 0) {
			printf("send example_data ok\n");
		}

		sleep(1);
	}

	close(m_sock);

	return 0;	
}


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/ntrip_util.h"


static const char kNtripStr[] = "STR;RTCM32;RTCM32;RTCM 3.2;"
                                "1004(1),1005/1007(5),PBS(10);2;GPS;SGNET;CHN;"
                                "31;121;1;1;SGCAN;None;B;N;0;;";

int main(int argc, char **argv) {
  int m_sock;
  int ret;
  char recv_buf[1024] = {0};
  char send_buf[1024] = {0};
  char userinfo_raw[48] = {0};
  char userinfo[64] = {0};

  char server_ip[] = "127.0.0.1";
  int server_port = 8090;
  // Mount point must be consistent with kNtripStr,
  // 'STR;{mountpoint};{mountpoint};'.
  char mountpoint[] = "RTCM32";
  char user[] = "test01";
  char passwd[] = "testing";

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  server_addr.sin_addr.s_addr = inet_addr(server_ip);

  // Generate base64 encoding of username and passwd.
  snprintf(userinfo_raw, 64 , "%s:%s", user, passwd);
  Base64Encode(userinfo_raw, userinfo);
  // Generate request example_data format of ntrip.
  snprintf(send_buf, 1023 ,
    "POST /%s HTTP/1.1\r\n"
    "Host: %s:%d\r\n"
    "Ntrip-Version: Ntrip/2.0\r\n"
    "User-Agent: %s\r\n"
    "Authorization: Basic %s\r\n"
    "Ntrip-STR: %s\r\n"
    "Connection: close\r\n"
    "Transfer-Encoding: chunked\r\n",
    mountpoint, server_ip, server_port,
    kServerAgent, userinfo, kNtripStr);

  unsigned char example_data[] = {
                    0xd3, 0x00, 0x70, 0x8e, 0x43, 0x56, 0x45, 0x00, 0x00,
                    0x55, 0xfb, 0x89, 0xff, 0xff, '\r', '\n'};

  if ((m_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    exit(1);
  }

  // Connect to caster.
  if (connect(m_sock, reinterpret_cast<struct sockaddr *>(&server_addr),
              sizeof(struct sockaddr_in)) < 0) {
    printf("connect to caster fail!!!\n");
    exit(1);
  }

  // Set socket no block.
  int flags = fcntl(m_sock, F_GETFL);
  fcntl(m_sock, F_SETFL, flags | O_NONBLOCK);

  // Send request data.
  if (send(m_sock, send_buf, strlen(send_buf), 0) < 0) {
    printf("send request fail!!!\n");
    exit(1);
  }

  // Wait for request to connect caster success.
  while (1) {
    memset(recv_buf, 0x0, 1024);
    ret = recv(m_sock, recv_buf, 1024, 0);
    if (ret > 0) {
      if (!strncmp(recv_buf, "ICY 200 OK\r\n", 12)) {
        break;
      }
    } else if (ret == 0) {
      printf("remote socket close!!!\n");
      break;
    }
  }

  // Send data to caster.
  while (1) {
    ret = send(m_sock, example_data, sizeof(example_data), 0);
    if (ret > 0) {
      printf("send example_data ok\n");
    } else if (ret == 0) {
      printf("remote socket close!!!\n");
      break;
    }
    sleep(1);
  }

  close(m_sock);
  return 0;
}


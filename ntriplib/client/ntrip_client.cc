#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/ntrip_util.h"


int main(int argc, char *argv[]) {
  int ret;
  int m_sock;
  time_t start, stop;
  char recv_buf[1024] = {0};
  char request_data[1024] = {0};
  char userinfo_raw[48] = {0};
  char userinfo[64] = {0};

  char server_ip[] = "127.0.0.1";
  int server_port = 8090;
  // If mount point is NULL, request source table.
  // char mountpoint[] = "";
  char mountpoint[] = "RTCM32";
  char user[] = "test01";
  char passwd[] = "testing";
  char gpgga[] = "$GPGGA,083552.00,3000.0000000,N,11900.0000000,E,"
                 "1,08,1.0,0.000,M,100.000,M,,*57\r\n";

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  server_addr.sin_addr.s_addr = inet_addr(server_ip);

  // Generate base64 encoding of username and passwd.
  snprintf(userinfo_raw, 63 , "%s:%s", user, passwd);
  Base64Encode(userinfo_raw, userinfo);

   // Generate request data format of ntrip.
  snprintf(request_data, 1023,
           "GET /%s HTTP/1.1\r\n"
           "User-Agent: %s\r\n"
           "Accept: */*\r\n"
           "Connection: close\r\n"
           "Authorization: Basic %s\r\n"
           "\r\n",
           mountpoint, kClientAgent, userinfo);

  m_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (m_sock == -1) {
    printf("create socket fail\n");
    exit(1);
  }

  // Connect to caster.
  if (connect(m_sock, reinterpret_cast<struct sockaddr *>(&server_addr),
              sizeof(struct sockaddr_in)) < 0) {
    printf("connect caster failed!!!\n");
    exit(1);
  }

  int flags = fcntl(m_sock, F_GETFL);
  fcntl(m_sock, F_SETFL, flags | O_NONBLOCK);

  // Send request data.
  if (send(m_sock, request_data, strlen(request_data), 0) < 0) {
    printf("send request failed!!!\n");
    exit(1);
  }

  // Wait for request to connect caster success.
  while (1) {
    ret = recv(m_sock, (void *)recv_buf, sizeof(recv_buf), 0);
    if ((ret > 0) && !strncmp(recv_buf, "ICY 200 OK\r\n", 12)) {
      ret = send(m_sock, gpgga, strlen(gpgga), 0);
      if (ret < 0) {
        printf("send gpgga data fail\n");
        exit(1);
      }
      printf("send gpgga data ok\n");
      break;
    } else if (ret == 0) {
      printf("remote socket close!!!\n");
      goto out;
    } else {
      printf("%s\n", recv_buf);
    }
  }

  // Receive data that returned by caster.
  start = time(NULL);
  while (1) {
    ret = recv(m_sock, recv_buf, sizeof(recv_buf), 0);
    if (ret > 0) {
      stop = time(NULL);
      printf("recv data:[%d] used time:[%d]\n", ret, (int)(stop - start));
      PrintChar(recv_buf, ret);
      start = time(NULL);
    } else if (ret == 0) {
      printf("remote socket close!!!\n");
      break;
    }
  }

out:
  close(m_sock);
  return  0;
}


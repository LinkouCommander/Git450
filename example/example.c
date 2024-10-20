/*
** showip.c -- 顯示命令列中所給的主機 IP address
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
  struct addrinfo hints, *res, *p;
  int status;
  char ipstr[INET6_ADDRSTRLEN];

  // 檢查使用方式是否正確
  if (argc != 2) {
    fprintf(stderr,"usage: showip hostname\n");
    return 1;
  }

  // 初始化 hints 結構
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  // 不用管是 IPv4 或 IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

  // 獲取主機資訊
  if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  printf("IP addresses for %s:\n\n", argv[1]);

  // 遍歷鏈結串列
  for(p = res; p != NULL; p = p->ai_next) {
    void *addr;
    char *ipver;

    // 取得 IP 位址
    if (p->ai_family == AF_INET) { // IPv4
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    } else { // IPv6
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }

    // 將 IP 轉換為字串並印出
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    printf(" %s: %s\n", ipver, ipstr);
  }

  freeaddrinfo(res); // 釋放鏈結串列
  return 0;
}

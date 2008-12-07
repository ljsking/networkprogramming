#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 1024
char *escapechar = "exit";
 
int main(int argc, char *argv[])
{
	char line[MAXLINE], sendline[MAXLINE], recvline[MAXLINE + 1];
	int n, size, comp;
	pid_t pid;
	static int s;
	static struct sockaddr_in server_addr;
	int port, ip;
	if(argc == 1){
		port = htons(1234);
		ip = inet_addr("127.0.0.1");
	}else if(argc != 3){
		port = htons(atoi(argv[2]));
		ip = inet_addr(argv[1]);
	}else{
		printf("use : %s server_IP port\n",argv[0]);
		exit(0);
	}
 
  s = socket(AF_INET, SOCK_STREAM,0);
 
	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = ip;
	server_addr.sin_port = port;
 
  	connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr));
  	printf("connect server\n");
  	if ((pid = fork()) > 0) {
		while (fgets(sendline, MAXLINE,stdin) != NULL) {
			char *ln = strstr(sendline, "\n");
			strncpy(line, sendline, ln-sendline);
			size = strlen(line);
			line[size++] = '\n';
			line[size++] = '\0';
			if (send(s, line, size, 0) != size)
				printf("error in size\n");
		}
		close(s);
		exit(0);
  } else if (pid == 0) {
    while (1) {
      if ((size = recv(s, recvline, MAXLINE, 0)) < 0) {
        printf("error read\n");
        close(s);
        exit(0);
      }
      recvline[size] = '\0';
      if (strstr(recvline, escapechar) != NULL)
        break;
      printf("%s", recvline);
    }
  }
  close(s);
}
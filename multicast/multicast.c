#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <string.h> 

#define MAXCHAR   1024 

int main(int argc, char *argv[]) 
{ 
	int n, ssock, rsock, pid, len, ttl = 32; 
	struct sockaddr_in sa; 
	struct ip_mreq mreq; 
	unsigned int value = 1; 
	struct sockaddr_in ssa; 
	char msg[MAXCHAR]; 

	if (argc != 3) { 
		printf("[%s]multicast_addr port\n", argv[0]); 
		exit(0); 
	} 
	printf("[%s]\n", argv[1]); 

	memset(&sa, 0, sizeof(sa)); 
	sa.sin_family = AF_INET; 
	sa.sin_addr.s_addr = inet_addr(argv[1]);  
	sa.sin_port = htons(atoi(argv[2])); 

	/* 수신 소켓 생성 */ 
	rsock = socket(AF_INET, SOCK_DGRAM, 0); 
	/* 멀티캐스트 그룹주소 및 인터페이스 주소설정 */ 
	mreq.imr_multiaddr = sa.sin_addr;  
	mreq.imr_interface.s_addr = htonl(INADDR_ANY); 

	/* 멀티캐스트 그룹에 가입 */ 
	setsockopt(rsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)); 
	/* 소켓 재 사용 옵션 */ 
	setsockopt(rsock, SOL_SOCKET,SO_REUSEADDR, &value, sizeof(value)); 

	bind(rsock,(struct sockaddr*)&sa,sizeof(sa)); 

	/* 송신용 소켓 개설 */ 
	ssock = socket(AF_INET, SOCK_DGRAM,0); 

	/* 전달할 패킷의 TTL값을 설정 */ 
	setsockopt(ssock,IPPROTO_IP,IP_MULTICAST_TTL, &ttl,sizeof(ttl)); 
	pid = fork(); 

	/* 자식 프로세스 : 수신 담당 */ 
	if (pid == 0) { 
	len = sizeof(ssa); 
	while (1) { 
		n = recvfrom(rsock, msg, MAXCHAR, 0, (struct sockaddr*)&ssa, &len); 
		if (n < 0) { 
			printf("recvfrom error\n"); 
			exit(0); 
		} else { 
			msg[n]=0; 
			printf("receive message : %s\n", msg); 
		} 
	} 
	/* 부모 프로세스 : 송신 담당 */ 
	} else if (pid > 0) { 
		char message[MAXCHAR]; 
		printf("send message : "); 
		while (fgets(message, MAXCHAR, stdin) != NULL) { 
			printf("%s\n", message); 

			if (sendto(ssock,message,strlen(message),0, 
					(struct sockaddr*)&sa,sizeof(sa)) < len) { 
				printf("sendto error\n"); 
				exit(0); 
			} else { 
				printf("send message : %s\n", message); 
			} 
		} 
	} else { 
		printf("fork() error\n"); 
		exit(0); 
	} 
} 

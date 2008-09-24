#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <signal.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
 
#define MAXLINE 512 
char *escapechar ="exit"; 
 
int main(int argc, char *argv[]) 
{ 
	fd_set master;   // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	int listener, newfd;
	int fdmax;
	int i, j;
	
	socklen_t addrlen;
	struct sockaddr_storage remoteaddr;
	
	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);
	
	int clilen, num; 
	char sendline[MAXLINE], recvline[MAXLINE]; 
	int size; 
	pid_t pid; 
	struct sockaddr_in client_addr, server_addr; 

	if (argc != 2) {
		printf("use : %s port\n", argv[0]); 
		exit(0); 
	} 

	listener = socket(AF_INET, SOCK_STREAM,0); 
	memset((char *)&server_addr, 0, sizeof(server_addr)); 

	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(atoi(argv[1])); 

	bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	listen(listener, 1); 
	// add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
	while(1)
	{
		read_fds = master; // copy it
		printf("copy it\n");
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(4);
		}
		printf("something happened %d\n", fdmax);
		for(i = 0; i<fdmax; i++){
			if(FD_ISSET(i, &read_fds)){
				printf("i is %d\n", i);
				if(i == listener){
					printf("it is listener!\n");
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
					printf("accepted %d\n", newfd);
					if(newfd == -1){
						perror("accept");
					}else{
						FD_SET(newfd, &master);
						if(newfd>fdmax){
							fdmax = newfd;
						}
						printf("new connection on socket %d\n", newfd);
					}
				}
			}else{
				if((size = recv(i, recvline, sizeof recvline, 0))<=0){
					if(size == 0)
					{
						printf("socket %d hung up\n", i);
					}else{
						perror("recv");
					}
					close(i);
					FD_CLR(i, &master);
				}else{
					for(j = 0; j<= fdmax; j++){
						if(FD_ISSET(j, &master)) {
							if(j != listener && j != i){
								if(send(j, recvline, size, 0)==-1){
									perror("send");
								}
							}
						}
					}
				}
			}
				
		}
	}
}
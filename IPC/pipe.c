#include <unistd.h> 
#include <stdio.h> 
 
#define MSGSIZE 16 
char *msg1 = "hello, world #1"; 
char *msg2 = "hello, world #2"; 
char *msg3 = "hello, world #3"; 
 
int main(void) 
{ 
 char inbuf[MSGSIZE]; 
 int p[2], j; 
  
 if (pipe(p) == -1) { /* 파이프를 개방한다 */ 
  perror("pipe call"); 
  exit(1); 
 } 
 
 /* 파이프에 쓴다 */ 
 write(p[1], msg1, MSGSIZE); 
 write(p[1], msg2, MSGSIZE); 
 write(p[1], msg3, MSGSIZE); 
 
 /* 파이프로부터 읽는다. */ 
 for (j = 0; j < 3; j++) { 
  read (p[0], inbuf, MSGSIZE); 
  printf ("%s\n", inbuf); 
 } 
 exit (0); 
} 

#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <unistd.h> 
 
int main(void) 
{ 
 int fp_w, fp_r, n; 
 char buf_r[11], buf_w[11]; 
 if ((fp_r = open("/tmp/fifo_c", O_RDONLY)) < 0) { 
  /* 시스템 에러 메세지를 stderr(표준에러)으로 출력한다. */ 
  perror("open error : "); 
  exit(1); 
 } 

 if ((fp_w = open("/tmp/fifo_s", O_WRONLY)) < 0) { 
  perror("open error : "); 
  exit(1); 
 } 

 memset(buf_r, 0x00, 11); 
 memset(buf_w, 0x00, 11); 

 /* fp_r에서 11바이트를 읽어 buf_r에 저장한다 */ 
 while ((n = read(fp_r, buf_r, 11)) > 0) { 
  /* 문자열을 정수로 바꾸어 buf_w에는 buf_r * buf_r 이 들어가게 된다. */ 
  sprintf(buf_w, "%d", atoi(buf_r) * atoi(buf_r)); 

  write(fp_w, buf_w, 11);  /* buf_w에 있는 데이터를 FIFO에 출력한다. */ 
  memset(buf_r, 0x00, 11); 
  memset(buf_w, 0x00, 11); 
 } 
 exit(0); 
} 

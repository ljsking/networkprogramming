#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <stdio.h>  
#include <unistd.h>  
int main(void) 
{ 
 int fp_r, fp_w, n, i; 
 char buf_r[11], buf_w[11]; 
 
 /* client로 데이터를 전송하기 위한 쓰기 전용의 FIFO 파일 개방 */ 
 if ((fp_w = open("/tmp/fifo_c", O_WRONLY)) < 0) { 
  perror("open error : "); 
  exit(1); 
 } 
 
 /* client로 부터 데이터를 받기 위한 읽기 전용의 FIFO 파일 개방 */ 
 if ((fp_r = open("/tmp/fifo_s", O_RDONLY)) < 0) { 
  perror("open error : "); 
  exit(1); 
 } 
 
 i = 1; 
 memset(buf_r, 0x00, 11); 
 memset(buf_w, 0x00, 11); 
 sprintf(buf_w, "%d", i);  
 
 while ((n = write(fp_w, buf_w, 11)) > 0) { 
  read(fp_r, buf_r, 11); 
 
  printf("%d : %d^2 = %s\n", i++, atoi(buf_w), buf_r); 
  memset(buf_r, 0x00, 11); 
  memset(buf_w, 0x00, 11); 
  sprintf(buf_w, "%d", i); 

  sleep(1); /* 터미널에서 문자열 입력을 기다리기 위해 1초 대기 */ 
 } 
} 

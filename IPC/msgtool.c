#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
 
#define MAX_SEND_SIZE 80 
struct mymsgbuf { 
 long mtype; 
 char mtext[MAX_SEND_SIZE]; 
}; 
 
void send_message(int qid, struct mymsgbuf *qbuf, long type, char *text); 
void read_message(int qid, struct mymsgbuf *qbuf, long type); 
void remove_queue(int qid); 
void change_queue_mode(int qid, char *mode); 
void usage(void); 
 
int main(int argc, char *argv[]) 
{ 
 key_t key; 
 int msgqueue_id; 
 struct mymsgbuf qbuf; 
 if (argc == 1) 
  usage(); 
  
 key = ftok(".", 'm');  /* ftok() 호출을 통해 유일한 키를 만든다 */ 
 /* 필요하다면 큐를 만들고 연다 */ 
 if ((msgqueue_id = msgget(key, IPC_CREAT|0660)) == -1) { 
  perror("msgget"); 
  exit(1); 
 } 
 switch (tolower(argv[1][0])) {
 case 's': 
  send_message(msgqueue_id, (struct mymsgbuf *)&qbuf, 
  atol(argv[2]), argv[3]);  
  break; 
 case 'r': 
  read_message(msgqueue_id, &qbuf, atol(argv[2]));  
  break; 
 case 'd': 
  remove_queue(msgqueue_id);
  break; 
 case 'm': 
  change_queue_mode(msgqueue_id, argv[2]);  
  break; 
 default: 
  usage(); 
 } 
 return(0); 
} 

void send_message(int qid, struct mymsgbuf *qbuf, long type, char *text) 
{ 
 /* 큐에 메세지를 보낸다 */ 
 printf("Sending a message ...\n"); 
 qbuf->mtype = type; 
 strcpy(qbuf->mtext, text); 
 if ((msgsnd(qid, (struct msgbuf *)qbuf, strlen(qbuf->mtext)+1, 0)) ==-1) { 
  perror("msgsnd"); 
  exit(1); 
 } 
} 

void read_message(int qid, struct mymsgbuf *qbuf, long type) 
{ 
 printf("Reading a message ...\n"); /* 큐로 부터 메세지를 읽는다. */ 
 qbuf->mtype = type; 
 msgrcv(qid, (struct msgbuf *)qbuf, MAX_SEND_SIZE, type, 0); 
 printf("Type: %ld Text: %s\n", qbuf->mtype, qbuf->mtext);
} 
 
void remove_queue(int qid) 
{ 
 /* 큐를 지운다 */ 
 msgctl(qid, IPC_RMID, 0); 
} 
void change_queue_mode(int qid, char *mode) 
{ 
 struct msqid_ds myqueue_ds; 
 
 /* 현재 정보를 읽는다 */ 
 msgctl(qid, IPC_STAT, &myqueue_ds); 
 
 sscanf(mode, "%ho", &myqueue_ds.msg_perm.mode);  
 msgctl(qid, IPC_SET, &myqueue_ds);  /* 모드를 수정한다 */ 
} 
 
void usage(void)  
{ 
 fprintf(stderr, "msgtool - A utility for tinkering with msg queues\n"); 
 fprintf(stderr, "\nUSAGE: msgtool (s)end  \n"); 
 fprintf(stderr, "(r)ecv \n"); 
 fprintf(stderr, "(d)elete\n"); 
 fprintf(stderr, "(m)ode \n"); 
 exit(1); 
} 

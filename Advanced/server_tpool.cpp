#include <map>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 최대 쓰레드 POOL 크기
#define MAX_THREAD_POOL 256
using namespace std;

// 전역 쓰레드 구조체 
typedef struct _ph
{
    int sockfd;    // 현재 사용중인 소켓 fd
    int index_num; // 인덱스 번호
} ph;

// 전역쓰레드 구조체로써 
// 현재 쓰레드 상황을 파악함
struct schedul_info
{
    int client_num;       // 현재 연결된 클라이언트수
    int current_sockfd;   // 가장최근에 만들어진 소켓지시자
    multimap<int, ph> phinfo; 
};

// 각 쓰레드별 조건변수
pthread_cond_t *mycond;
// 쓰레드 동기화를 위한 조건변수
pthread_cond_t async_cond = PTHREAD_COND_INITIALIZER;

// 각 쓰레드별 조건변수의 크리티컬세션 지정을 위한 
// 뮤텍스 
pthread_mutex_t mutex_lock= PTHREAD_MUTEX_INITIALIZER; 
// 쓰레드 동기화용 조건변수의 크리티컬세션 지정을 위한 
// 뮤텍스
pthread_mutex_t async_mutex = PTHREAD_MUTEX_INITIALIZER;

// 클라이언트와의 통신용 쓰레드
void *thread_func(void *data);
// 현재 클라이언트 상태 모니터용 쓰레드
// 한마디로 디버깅용 
void *mon_thread(void *data);

schedul_info s_info;

// 메인 함수
int main(int argc, char **argv)
{
    int i;
    ph myph;
    int status;
    int pool_size = atoi(argv[2]);
    pthread_t p_thread;
    struct sockaddr_in clientaddr, serveraddr;
    int server_sockfd;
    int client_sockfd;
    int client_len;    

    // 풀사이즈 검사
    if ((pool_size < 0) || (pool_size > MAX_THREAD_POOL))
    {    
        cout << "Pool size Error" << endl;
        exit(0);    
    }

    // Make Socket
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("error : ");
        exit(0);
    }

    // Bind
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[1]));

    if (bind (server_sockfd, (struct sockaddr *)&serveraddr, 
                        sizeof(serveraddr)) == -1) 
    {
        perror("bind error : ");
        exit(0);
    }

    // Listen
    if (listen(server_sockfd, 5) == -1)
    {
        perror("listen error : ");
        exit(0);
    }

    // 쓰레드 갯수만큼 조건변수 생성 
    mycond     = (pthread_cond_t *)malloc(sizeof(pthread_cond_t)*pool_size);

	
	for(i=0;i<pool_size;i++)
		pthread_cond_init(&mycond[i],NULL);

    // 쓰레드 전역변수 초기화
    s_info.client_num = 0;
    s_info.current_sockfd = 0; 

    // 쓰레드 POOL 새성
    for (i = 0; i < pool_size; i++)
    {
        memset((void *)&myph, 0x00, sizeof(myph));
        myph.index_num = i;
        s_info.phinfo.insert(pair<int, ph>(0, myph));

        // 조건변수를 이용해서 쓰레드간 동기화를 실시한다.
        pthread_mutex_lock(&async_mutex);
        if (pthread_create(&p_thread, NULL, thread_func, (void *)&i) < 0)
        {
            perror("thread Create error : ");
            exit(0);    
        }    
        pthread_cond_wait(&async_cond, &async_mutex); 
        pthread_mutex_unlock(&async_mutex);
    }

    // 디버깅용 쓰레드 생성
    pthread_create(&p_thread, NULL, mon_thread, (void *)NULL);

    // MAIN THREAD accept wait
    client_len = sizeof(clientaddr);

    // 클라이언트 ACCEPT 처리를 위한 
    // MAIN 쓰레드 
    while(1)
    {
        multimap<int, ph>::iterator mi;
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, 
                                        (socklen_t *)&client_len);
		cout<<"client sock "<<client_sockfd<<endl;
        if (client_sockfd > 0)
        {
            // 만약 쓰레드풀이 가득찼다면 클라이언트 연결을
            // 종료시킨다.
            mi = s_info.phinfo.begin();
            if (mi->first == 1)
            {
                //error message send to client_sockfd
                cout << "SOCKET IS FULL" << endl;    
                close(client_sockfd);
            }
            // 그렇지 않다면 연결을 받아들이고 
            // 클라이언트 전역 변수를 세팅한다. 
            // 세팅후 해당 처리쓰레드에게 시그널을 보내어서 
            // 처리하게 한다. 
            else
            {
                ph tmpph;
                int psockfd;
                int pindex_num;
                s_info.current_sockfd = client_sockfd;

                tmpph.sockfd = client_sockfd;
                tmpph.index_num = mi->second.index_num;
                s_info.phinfo.erase(mi);    
                s_info.phinfo.insert(pair<int, ph>(1,tmpph));
                s_info.client_num ++;
                cout << "SEND SIGNAL " << mi->second.index_num << endl;     
                pthread_cond_signal(&mycond[mi->second.index_num]);    
            }
        }
        else
        {
            cout << "ACCEPT ERROR " << endl;    
        }
    }
    pthread_join(p_thread, (void **)status);
}

void *thread_func(void *data)
{
	const int MAXDATASIZE = 255;
    char buf[MAXDATASIZE];
    int mysocket;
    int mynum = *((int *)data); 
    multimap<int, ph>::iterator mi;
    // 쓰레드 동기화용 조건변수
    pthread_mutex_lock(&async_mutex);
    pthread_cond_signal(&async_cond);
    pthread_mutex_unlock(&async_mutex);

    cout << "Thread create " << mynum << endl;
    while(1)
    {
        // MAIN 쓰레드로 부터 신호를 기다린다. 
        // 신호가 도착하면 쓰레드 전역변수로 부터 
        // 현재 처리해야할 소켓지정값을 가져온다. 
        pthread_mutex_lock(&mutex_lock);
		cout<<"mutex_lock "<<mynum<<endl;
        pthread_cond_wait(&mycond[mynum], &mutex_lock);
        mysocket = s_info.current_sockfd;
		cout<<"mysocket:"<<mysocket<<endl;
        pthread_mutex_unlock(&mutex_lock);
        memset(buf, 0x00, 255);    

        // 데이타를 처리한다. 
        // 만약 quit 문자열을 만나면 
        // 쓰레드 전역변수를 세팅한다음 연결종료 한다. 
        while(1)
        {
			cout<<"wait receive"<<endl;
			int numbytes;
            if ((numbytes = recv(mysocket, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
		        exit(1);
		    }
		    buf[numbytes] = '\0';
		    cout<<"client: received "<<buf;
            if (strstr(buf, "quit") == NULL)
            {
                write(mysocket, buf, 255);
            }
            else
            {
                mi = s_info.phinfo.begin();
                while(mi != s_info.phinfo.end())
                {
                    cout << "search " << mi->second.index_num << endl;
                    if (mi->second.index_num == mynum)
                    {
                        ph tmpph;
                        tmpph.index_num = mynum;
                        tmpph.sockfd = 0;
                        s_info.phinfo.erase(mi);
                        s_info.phinfo.insert(pair<int, ph>(0, tmpph));
                        s_info.client_num --;
                        close(mysocket);
                        break;
                    }
                    mi ++;
                }
                break;
            }
            memset(buf, 0x00, 255);    
        }
    }
}

void *mon_thread(void *data)
{
    cout << "moniter thread" << endl;
    while(1)
    {
        sleep(10);
        multimap<int, ph>::iterator mi;
        mi = s_info.phinfo.begin();
        cout << "size " << s_info.phinfo.size() << endl;
        while(mi != s_info.phinfo.end())
        {
            cout << mi->first << " : " << mi->second.index_num 
                 << " : " << mi->second.sockfd << endl; 
            mi ++;
        }
    }
}

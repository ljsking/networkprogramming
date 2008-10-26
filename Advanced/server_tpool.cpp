#include <map>
#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>


using namespace std;

//thread info structure
struct ph
{
    int sockfd;    // number of socket 
    int index_num; // index number 
};

// thread structure MAP
typedef multimap<int, struct ph> phinfo;

struct schedule_info
{
    int client_num;      // total number of connected clients
    int current_sockfd;  // number of a lastest connected socket
    phinfo mphinfo;      // thread structure map
};

// conditional variable for each thread
pthread_cond_t *mycond;
// conditional variable for thread synchronization
pthread_cond_t async_cond = PTHREAD_COND_INITIALIZER;
// mutex for critical section of a conditional variable of each thread
pthread_mutex_t mutex_lock= PTHREAD_MUTEX_INITIALIZER; 
// mutex for critical section of thread synchronization
pthread_mutex_t async_mutex = PTHREAD_MUTEX_INITIALIZER;
// thread for a conmmunication of client
void *thread_func(void *data);
// thread for monitoring current client status
void *mon_thread(void *data);

schedule_info s_info;
const int max_thread_pool = 256;

void usage(void);

int main(int argc, char **argv)
{
    if(argc != 3)
        usage();
    int i;
    ph myph;
    int status;
    int pool_size = atoi(argv[2]);
    pthread_t p_thread;
    struct sockaddr_in clientaddr, serveraddr;
    int server_sockfd;
    int client_sockfd;
    int client_len;

    //test of pool size
    if((pool_size < 0) || (pool_size > max_thread_pool))
    {
        cout<<"Pool size error"<<endl;
        exit(-1);
    }

    //make socket
    if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)<0))
    {
        perror("server socket:");
        exit(-1);
    }

    //bind
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[1]));

    if(bind(server_sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("bind:");
        exit(-1);
    }

    if(listen(server_sockfd, 5) == -1)
    {
        perror("listen:");
        exit(0);
    }

    //make condional variables as number of threads
    mycond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t)*pool_size);

    //initialize global thread variables
    s_info.client_num = 0;
    s_info.current_sockfd = 0;

    //make thread_pool
    for(i = 0; i<pool_size; i++)
    {
        memset((void*)&myph, 0x00, sizeof(myph));
        myph.index_num = i;
        s_info.mphinfo.insert(pair<int, ph>(0, myph));

        //synchronize between threads using conditional varialbe
        pthread_mutex_lock(&async_mutex);
        if(pthread_create(&p_thread, NULL, thread_func, (void *)&i)<0)
        {
            perror("Thread create error:");
            exit(-1);
        }

        //synchronize between threads using conditional varialbe
        pthread_mutex_lock(&async_mutex);
        if(pthread_create(&p_thread, NULL, thread_func, (void *)&i)<0)
        {
            perror("Thread create error:");
            exit(-1);
        }
        pthread_cond_wait(&async_cond, &async_mutex);
        pthread_mutex_unlock(&async_mutex);

        //create debugging thread
        pthread_create(&p_thread, NULL, mon_thread, (void *)NULL);

        //main thread for accepting client
        while(1)
        {
            multimap<int, ph>::iterator mi;
            client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, (socklen_t*)&client_len);
            if(client_sockfd>0)
            {
                //disconnect connection if thread pool is full
                mi = s_info.mphinfo.begin();
                if(mi->first == 1)
                {
                    cout<<"thread pool is full"<<endl;
                    close(client_sockfd);
                }
                else
                {
                    ph tmpph;
                    int psockfd;
                    int pindex_num;
                    s_info.current_sockfd = client_sockfd;

                    tmpph.sockfd = client_sockfd;
                    tmpph.index_num = mi->second.index_num;
                    s_info.mphinfo.erase(mi);
                    s_info.mphinfo.insert(pair<int, ph>(1, tmpph));
                    s_info.client_num++;
                    cout<<"Send signal"<<mi->second.index_num<<endl;
                    pthread_cond_signal(&mycond[mi->second.index_num]);
                }
            }
            else
                cout<<"Accept error"<<endl;
        }
    }
}

void usage()
{
    cout<<"server port max_thread_pool"<<endl;
    exit(-1);
}

void *thread_func(void *data)
{
	char buf[255];
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
        pthread_cond_wait(&mycond[mynum], &mutex_lock);
        mysocket = s_info.current_sockfd;
        pthread_mutex_unlock(&mutex_lock);
        memset(buf, 0x00, 255);    

        // 데이타를 처리한다. 
        // 만약 quit 문자열을 만나면 
        // 쓰레드 전역변수를 세팅한다음 연결종료 한다. 
        while(1)
        {
            read(mysocket, buf, 255);
            if (strstr(buf, "quit") == NULL)
            {
                write(mysocket, buf, 255);
            }
            else
            {
                mi = s_info.mphinfo.begin();
                while(mi != s_info.mphinfo.end())
                {
                    cout << "search " << mi->second.index_num << endl;
                    if (mi->second.index_num == mynum)
                    {
                        ph tmpph;
                        tmpph.index_num = mynum;
                        tmpph.sockfd = 0;
                        s_info.mphinfo.erase(mi);
                        s_info.mphinfo.insert(pair<int, ph>(0, tmpph));
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

void *mon_thread(void *arg)
{
    cout << "moniter thread" << endl;
    while(1)
    {
        sleep(10);
        multimap<int, ph>::iterator mi;
        mi = s_info.mphinfo.begin();
        cout << "size " << s_info.mphinfo.size() << endl;
        while(mi != s_info.mphinfo.end())
        {
            cout << mi->first << " : " << mi->second.index_num 
                 << " : " << mi->second.sockfd << endl; 
            mi ++;
        }
    }
}

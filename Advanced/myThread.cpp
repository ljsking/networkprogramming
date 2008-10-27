
#include <string.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <qapplication.h>

#include "myThread.h"
MyThread::MyThread()
{
}

void MyThread::run()
{
	char buff[256];
    while(true) {
        sleep( 1 );
        msgrcv(msgqueue_id, (struct msgbuf *)buff, 256, 0, 0);
		edit->append(buff);
    }
}
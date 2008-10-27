#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <QThread>
#include <QTextEdit>

class MyThread : public QThread {
public:
	MyThread();
    virtual void run();
	int msgqueue_id;
	QTextEdit *edit;
};
#endif

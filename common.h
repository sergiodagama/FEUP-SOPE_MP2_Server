#ifndef _COMMON_H
#define _COMMON_H 1
typedef struct {
	int rid; 										// request id
	pid_t pid; 										// process id
	pthread_t tid;									// thread id
	int tskload;									// task load
	int tskres;										// task result
} Message;
#endif // _COMMON_H

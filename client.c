/*
 *	MP2 - client.c - versão 5.mai.2021
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include "common.h"

#define CONFORTSIZE	1024
#define PERM 0666	// permissões de acesso ao FIFO

int timeout = 0;				// execution timeout, changed by command line args
char *serverfifoname = NULL;	// will be kept in command line args
int serverfifo = -1;			// fifo for incoming requests

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;	// mutex for accessing the counter (Nuno!)
int counter = 0;				// counter of launched threads, serves as unique request id
int finish = 0;					// whether time to finish

void get_opt(int , char* []);
void alrm(int);
void pips(int);
void terminate_blocked (pid_t);
void cleanup_handler(void *);

// thread that launches a request to the server and waits for reply
void* thread_client(void* arg) {
		int clientfifo = -1;
		char clientfifoname[CONFORTSIZE];
	// create a unique fifo to get back the answer
	sprintf(clientfifoname, "/tmp/%d.%lu", getpid(), (unsigned long) pthread_self());
	if (mkfifo(clientfifoname, PERM) < 0)  {
		perror ("mkfifo clientfifoname");
		pthread_exit(NULL);
	}
	fprintf(stderr,"[client] created client fifo %s\n",clientfifoname);

	// generate unique request id
	pthread_mutex_lock(&mut);
	int	rid = counter++;
	pthread_mutex_unlock(&mut);
	// generates a random task load
	int t = (rand() % 9) + 1;

	Message *request = malloc(sizeof(Message));
	if (request == NULL) {
		perror ("malloc(sizeof(Message)");
		unlink(clientfifoname);
		pthread_exit(NULL);
	}

	request->rid = rid;
	request->tskload = t;
	request->pid = getpid();
	request->tid = pthread_self();
	request->tskres = -1;

	// serverfifo was opened in main() ; write() should not block...
	if (write(serverfifo,request,sizeof(Message)) < 0) {
		perror("[client] write serverfifo");
		free(request);
		unlink(clientfifoname);
		pthread_exit(NULL);
	}

	printf("%ld ; %d ; %d ; %d ; %lu ; %d; IWANT\n", time(NULL), request->rid, request->tskload, request->pid, (unsigned long) request->tid, request->tskres);

 	pthread_cleanup_push(cleanup_handler, request);	// for the case of cancelling

	if((clientfifo = open(clientfifoname, O_RDONLY)) < 0) {	// open blocks; break it by cancelling the thread
		perror ("open clientfifo");
		printf("%ld ; %d ; %d ; %d ; %lu ; %d; GAVUP\n", time(NULL), request->rid, request->tskload, request->pid, (unsigned long) request->tid, request->tskres);
		free(request);
		unlink(clientfifoname);
		return (NULL);
	}

 	fprintf(stderr,"[client] opened client fifo %s\n", clientfifoname);

		int nread;
		Message answer;
	if ((nread = read(clientfifo, (void *)&answer, sizeof(Message))) < 0) {	// read blocks; break it by cancelling the thread
		perror("[client] read(clientfifo)");
		printf("%ld ; %d ; %d ; %d ; %lu ; %d; GAVUP\n", time(NULL), request->rid, request->tskload, request->pid, (unsigned long) request->tid, request->tskres);
		}
	else if (nread == 0) {	// server closed private FIFO
		fprintf(stderr,"[client] server closed client fifo %s\n", clientfifoname);
		printf("%ld ; %d ; %d ; %d ; %lu ; %d; GAVUP\n", time(NULL), request->rid, request->tskload, request->pid, (unsigned long) request->tid, request->tskres);
		}
	else if (answer.tskres < 0) {
		printf("%ld ; %d ; %d ; %d ; %lu ; %d; CLOSD\n", time(NULL), request->rid, request->tskload, request->pid, (unsigned long) request->tid, answer.tskres);
		}
	else {
		printf("%ld ; %d ; %d ; %d ; %lu ; %d; GOTRS\n", time(NULL), request->rid, request->tskload, request->pid, (unsigned long) request->tid, answer.tskres);
		}

 	pthread_cleanup_pop(0);	// disable cancelling functions
	free(request);
	close(clientfifo);
	unlink(clientfifoname);
	return(NULL);
} // thread_client()

int main(int argc, char** argv) {
	get_opt(argc,argv);

	if (timeout == 0) {
		fprintf(stderr, "[server] no timeout set - exiting\n");
		exit(1);
	}
		struct sigaction new, old;
		sigset_t smask;	// signals to mask during signal handler
	sigemptyset(&smask);
	new.sa_handler = alrm;
	new.sa_mask = smask;
	new.sa_flags = 0;	// usually enough
	if(sigaction(SIGALRM, &new, &old) == -1) {
		perror ("sigaction (SIGALRM)");
		exit(2);
	}
		struct sigaction new2;
	new2.sa_handler = pips;
	new2.sa_mask = smask;
	new2.sa_flags = 0;	// usually enough
	if(sigaction(SIGPIPE, &new2, NULL) == -1) {
		perror ("sigaction (SIGPIPE)");
		exit(2);
	}

	alarm(timeout);	// this will flip the "finish" flag

	while ((serverfifo = open(serverfifoname, O_WRONLY)) < 0) {	// 1st time: keep blocking until server opens...
		perror("[client] client, open serverfifo");
		if (finish)	// client timeout!
			pthread_exit(NULL);
	}

	srand(time(NULL));	// seed the task "loads"
	pthread_t tid;	// temporary, for any of the client threads

	// launches requests until client times out
	while (1) {

		while (pthread_create(&tid, NULL, thread_client, NULL) != 0) {	// wait till a new thread can be created!
			perror("[client] server thread");
			usleep(10000 + (rand() % 10000));
			if (finish)	// client timeout!
				goto timetoclose;
		}
		// sleeps a random time until next request
		usleep(10000 + (rand() % 10000));	// prepare new request
// 		usleep(100 + (rand() % 100));	// good for forcing 2LATEs !

		if (access(serverfifoname, F_OK) < 0) { // if server removed public FIFO
			close (serverfifo);
			do { // busy wait until server re-opens or client times out
				if (finish)	// client timeout!
					goto timetoclose;
			} while (access(serverfifoname, F_OK) < 0);	// busy wait until server re-opens or client times out
			// server should have re-opened
			if ((serverfifo = open(serverfifoname, O_WRONLY)) < 0) {	// keep blocking until server opens... OR intr (e.g. alarm) occurs
				perror("[client] client, open serverfifo");
				goto timetoclose;	// could be error or timeout
			} // open(serverfifoname)
		} // if(access())

		if (finish)	// client timeout!
			goto timetoclose;
	} // while(1)

timetoclose:
	fprintf(stderr, "[client] stopped creating requests\n");
	// strategy: 1 2
	// 1 - break all blocked threads with pthread_cancel() and assure GAVEUP messages
		// como saber os thr_ids? percorrendo /tmp/[pid].* !
	terminate_blocked(getpid());
	// 2 - assure that all private FIFOs are removed
	char rm[256] = {'\0'};
	snprintf (rm, sizeof (rm), "rm -rf /tmp/%d.*", getpid());
	system (rm);

	fprintf(stderr, "[client] main terminating\n");
	pthread_exit(NULL);
} // main()

void get_opt(int argc, char* argv[]) {
	if(argc < 3) {
		fprintf(stderr, "Usage: %s <-t nsecs> <fifoname>\n", argv[0]);
		exit(1);
	}
	int opt;
	while ((opt = getopt(argc, argv, "t:")) != -1) {
		switch (opt) {
			case 't':
				timeout = atoi(optarg);
				fprintf(stderr, "Initial time: %lu, expected final time: %lu\n",time(NULL),time(NULL)+timeout);
				break;
			default:
				fprintf(stderr, "Usage: %s <-t nsecs> <fifoname>\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
		if (optind != argc-1) {
		fprintf(stderr, "Usage: %s <-t nsecs> <fifoname>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	else
		serverfifoname = argv[optind];
	fprintf(stderr,"\nGot: nsecs=%d, fifoname=%s\n", timeout, serverfifoname);
} // get_opt

void alrm(int signo) {
	finish = 1;
	fprintf(stderr, "[client] timeout reached: %ld %ld\n", time(NULL), (unsigned long) pthread_self());
} // alrm()

void pips(int signo) {
	finish = 1;
	fprintf(stderr, "[client] server pipe closed\n");
} // pips()

void terminate_blocked (pid_t pid) {
		DIR *od;
		struct dirent *ds;
		char fifoname [1024] = {'\0'};
		char threadid [1024] = {'\0'};
		pthread_t tid;
	sprintf(fifoname, "%d.", pid);

	if ((od = opendir("/tmp")) == NULL) {
		perror("opendir");
	}
	while((ds = readdir(od)) != NULL) {
		char *ptr;
		if ((ptr = strstr(ds->d_name, fifoname)) != NULL) {
			ptr = ptr + strlen(fifoname);
			sprintf(threadid, "%s", ptr);
			tid = (unsigned long) atoll(threadid);
			pthread_cancel(tid);
			fprintf(stderr, "[client] cancelled thread %lu.\n", tid);
		}
	}
} //terminate_blocked()

void cleanup_handler(void *arg) {
		Message *request = (Message *) arg;
	fprintf(stderr, "[Client] Called clean-up handler\n");
	printf("%ld ; %d ; %d ; %d ; %lu ; %d; GAVUP\n", time(NULL), request->rid, request->tskload, request->pid, (unsigned long) request->tid, request->tskres);
} // cleanup_handler()

#ifndef __signal__
#define __signal__
#define SIGHUP	1
#define SIGINT	2
#define SIGQUIT	3
#define SIGILL	4
#define SIGTRAP	5
#define SIGABRT	6
#define SIGFPE	8
#define SIGKILL	9

int signal(int SIGNAL, void (*handler)());
int sentSignal(int pid, int SIGNAL);
void* sig_handler_kernel(struct thread *t);
void* sig_handler_assembly(void (*func)(), void *sp_addr, void* args);
#endif
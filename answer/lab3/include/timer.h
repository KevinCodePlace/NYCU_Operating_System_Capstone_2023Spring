#ifndef __TIMER_H
#define __TIMER_H
#define CORE0_TIMER_IRQ_CTRL ((volatile unsigned int *)(0x40000040))
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(0x40000060))

typedef void (*timer_callback)(char *);

void core_timer_enable();
void core_timer_disable();
void set_expired_time(unsigned long duration);
unsigned long get_current_time();
void core_timer_handler();
void set_timeout(char *message, char *_time);
void print_message(char *msg);
void add_timer(timer_callback cb,char *msg, unsigned long duraction);
void timeout_event_init();
void timer_handler(void *arg);

typedef struct timeout_event {
  unsigned long register_time;
  unsigned long duration;
  timer_callback callback;
  char msg[20];
  struct timeout_event *prev, *next;
} timeout_event;

extern timeout_event *timeout_queue_head, *timeout_queue_tail;

#endif

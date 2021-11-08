#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/user/syscall.h"

void syscall_init (void);

/* CSE4070 implementation */
void halt (void);
void exit (int);
pid_t exec (const char *);
int wait (pid_t);
int read (int, void *, unsigned);
int write (int, const void *, unsigned);
int fibonacci (int);
int max_of_four_int (int, int, int, int);

#endif /* userprog/syscall.h */

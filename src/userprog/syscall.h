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
bool create (const char *, unsigned);
bool remove (const char *);
int open (const char *);
int filesize (int);
void seek (int, unsigned);
unsigned tell (int);
void close (int);
int fibonacci (int);
int max_of_four_int (int, int, int, int);

#endif /* userprog/syscall.h */

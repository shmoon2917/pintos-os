#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  uint32_t syscall_number;

  if (!is_user_vaddr(f->esp)) {
    exit(-1);
  }

  syscall_number = *(uint32_t*)(f->esp);
  switch (syscall_number){
    case SYS_HALT: {
      halt();
	    break;
    }  
    case SYS_EXIT: {
	    if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }
      
      exit(*(uint32_t *)(f->esp + 4));
	    break;
    }
    case SYS_EXEC: {
      if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = exec(*(char**)(f->esp + 4));
      break;
    }
    case SYS_WAIT: {
      if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = wait(*(int*)(f->esp + 4));
      break;
    }
    case SYS_READ: {
      if (!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12)) {
        exit(-1);
      }

      f->eax = read((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8),(unsigned)*(uint32_t*)(f->esp + 12));
      break;
    }
    case SYS_WRITE: {
      if (!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12)) {
        exit(-1);
      }

      f->eax = write((int)*(uint32_t*)(f->esp + 4),(void*) *(uint32_t*)(f->esp + 8),(unsigned)*(uint32_t*)(f->esp + 12));
      break;
    }
    case SYS_FIBONACCI: {
      if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = fibonacci((int)*(uint32_t*)(f->esp + 4));
      break;
    }
    case SYS_MAX_OF_FOUR_INT: {
      if (!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12) || !is_user_vaddr(f->esp + 16)) {
        exit(-1);
      }

      f->eax = max_of_four_int((int)*(uint32_t*)(f->esp + 4), (int)*(uint32_t*)(f->esp + 8), (int)*(uint32_t*)(f->esp + 12), (int)*(uint32_t*)(f->esp + 16));
      break;
    }
    case SYS_CREATE: {
      f->eax = create(*(char **)(f->esp + 4), *(unsigned *)(f->esp+ 8 ));
      break;
    }
    case SYS_REMOVE: {
      f->eax = remove(*(char **)(f->esp + 4));
	    break;
    }
    case SYS_OPEN: {
      if(!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = open(*(char**)(f->esp + 4));
      break;
    }
    case SYS_CLOSE: {
    	if(!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

	    close(*(uint32_t *)(f->esp + 4));
      break;
    }
    case SYS_FILESIZE: {
      f->eax = filesize(*(uint32_t *)(f->esp + 4));
      break;
    }
    case SYS_SEEK: {
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8)) {
        exit(-1);
      }

      seek((int)*(uint32_t*)(f->esp + 4), (unsigned)*(uint32_t*)(f->esp + 8)); 
	    break;
    }
    case SYS_TELL: {
      if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = tell((int)*(uint32_t*)(f->esp + 4));
      break;
    }
  }
}

void halt (void){
	shutdown_power_off();
}

void exit(int status){
	struct thread* curr, * parent_thread;
	int parent_tid;
	curr = thread_current();

  printf("%s: exit(%d)\n", curr->name, status);

  parent_tid = curr->parent_tid;
	parent_thread = get_thread_by_tid(parent_tid);
	parent_thread->exit_status = status;
	
	thread_exit();
}

pid_t exec (const char *cmd_line)
{
 return process_execute(cmd_line);
}

int wait (pid_t pid){
	return process_wait(pid);
}

int read (int fd, void *buffer, unsigned size)
{
  int i;
  void *buff = buffer;

  if (fd == 0) {
    for (i = 0; i < (int)size; i++) {
      *(uint8_t *)buff = input_getc();
      if (*(uint8_t *)buff == '\0') {
        break;
      }

      *(uint8_t *)buff += 1;
    }

    return i;
  }

  return -1;
}

int write(int fd, const void *buffer, unsigned size){
	if (fd == 1) {
		putbuf(buffer, size);
		return size;
	}

	return -1;
}

int fibonacci (int n)
{
  if (n <= 0) {
    return 0;
  } else if (n == 1) {
    return 1;
  }

  return fibonacci(n - 2) + fibonacci(n - 1);
}

int max_of_four_int (int n1, int n2, int n3, int n4){
  int temp1, temp2;
  temp1 = n1 >= n2 ? n1 : n2;
  temp2 = n3 >= n4 ? n3 : n4;

	return temp1 >= temp2 ? temp1 : temp2;
}

bool create (const char *file, unsigned initial_size) {
  if (!file) {
    exit(-1);
  }

  return filesys_create(file, initial_size);
}

bool remove (const char *file) {
  if (!file) {
    exit(-1);
  }

  return filesys_remove(file);
}

int open (const char *file) {
	if(!file) exit(-1);

	struct thread *cur;
	struct file *open_file;
	int i;

	cur = thread_current();
	open_file = filesys_open(file);
	if(open_file == NULL) {
		return -1;
	}

	if(!strcmp(thread_name(), file)) file_deny_write(open_file);
	for(i = 2; i < 128; i++) {
		if(cur->files[i] == NULL) {
			cur->files[i] = open_file;
			cur->file_cnt++;
			return i;
		}
	}
	return -1;
}

void close (int fd) {
  struct thread *cur;
	cur = thread_current();

	if(cur->files[fd] != NULL) {
		cur->file_cnt--;
		file_close(cur->files[fd]);
		cur->files[fd] = NULL;
	}
}

int filesize (int fd) { 
  return file_length(thread_current()->files[fd]);
}

void seek (int fd, unsigned position) {
  struct file *curr_file = thread_current()->files[fd];
  file_seek(curr_file, position);
}

unsigned tell (int fd) {
  struct file *curr_file = thread_current()->files[fd];
  return file_tell(curr_file);
}


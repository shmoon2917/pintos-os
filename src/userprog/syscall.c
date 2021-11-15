#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

// global lock
struct lock lock;

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
  // initialize lock
  lock_init(&lock);

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

      f->eax = exec(*(char **)(f->esp + 4));
      break;
    }
    case SYS_WAIT: {
      if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = wait(*(int *)(f->esp + 4));
      break;
    }
    case SYS_CREATE: {
      if (!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8)) {
        exit(-1);
      }

      f->eax = create(*(char **)(f->esp + 4), *(unsigned *)(f->esp + 8));
      break;
    }
    case SYS_REMOVE: {
      if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = remove(*(char **)(f->esp + 4));
	    break;
    }
    case SYS_FILESIZE: {
      if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }
      
      f->eax = filesize(*(uint32_t *)(f->esp + 4));
      break;
    }
    case SYS_SEEK: {
      if (!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8)) {
        exit(-1);
      }

      seek((int)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 8)); 
	    break;
    }
    case SYS_TELL: {
      if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = tell((int)*(uint32_t *)(f->esp + 4));
      break;
    }
    case SYS_OPEN: {
      if(!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

      f->eax = open(*(char **)(f->esp + 4));
      break;
    }
    case SYS_CLOSE: {
    	if(!is_user_vaddr(f->esp + 4)) {
        exit(-1);
      }

	    close(*(uint32_t *)(f->esp + 4));
      break;
    }
    case SYS_READ: {
      if (!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12)) {
        exit(-1);
      }

      f->eax = read((int)*(uint32_t *)(f->esp + 4), (void*) *(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
      break;
    }
    case SYS_WRITE: {
      if (!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12)) {
        exit(-1);
      }

      f->eax = write((int)*(uint32_t *)(f->esp + 4), (void*) *(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
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
  }
}

void halt (void) {
	shutdown_power_off();
}

void exit(int status) {
	struct thread* curr = thread_current();
  curr->exit_status = status;
  // curr->load_status = 0;

  printf("%s: exit(%d)\n", curr->name, status);

  // file close
  for (int i = 2; i < 256; i++) {
		if (curr->files[i] != NULL) 
      close(i);
  }
	
	thread_exit();
}

pid_t exec (const char *cmd_line) {
 return process_execute(cmd_line);
}

int wait (pid_t pid) {
	return process_wait(pid);
}

int read (int fd, void *buffer, unsigned size) {
  if(!(fd >= 0 && fd < 256) || !is_user_vaddr(buffer + size)) {
    exit(-1);
  }

  // locking 처리
  lock_acquire(&lock);

  // STDIN 읽어오는 부분
  if (fd == 0) {
    
    int i;
    for (i = 0; i < (int)size; i++) {
      ((uint8_t *)buffer)[i] = input_getc();
      if (((uint8_t *)buffer)[i] == 0) {
        break;
      }

      // *(uint8_t *)buff += 1;
    }

    // unlocking 처리
    lock_release(&lock);
    return i;
  } else {
		struct file *file = thread_current()->files[fd];

		if (file == NULL) {
      // unlocking 처리
      lock_release(&lock);
			exit(-1);
      // return
		}
	
		int byte = file_read(file, buffer, size);

    // unlocking 처리
		lock_release(&lock);

		return byte;
  }
}

int write (int fd, const void *buffer, unsigned size) {
  if(!(fd >= 0 && fd < 256) || !is_user_vaddr(buffer + size)) {
    exit(-1);
  }

  // locking 처리
  lock_acquire(&lock); 

  // STDOUT
	if (fd == 1) {
		putbuf(buffer, size);

		return size;
	} else if (fd >= 2 && fd < 256) {
		struct file *file = thread_current()->files[fd];

		if (file == NULL) {
			// unlocking 처리
      lock_release(&lock);
			exit(-1);
		}
	
		int byte = file_write(file, buffer, size);
    // unlocking 처리
		lock_release(&lock);
		return byte;
  }

	return -1;
}

int fibonacci (int n) {
  if (n <= 0) {
    return 0;
  } else if (n == 1) {
    return 1;
  }

  return fibonacci(n - 2) + fibonacci(n - 1);
}

int max_of_four_int (int n1, int n2, int n3, int n4) {
  int temp1, temp2;
  temp1 = n1 >= n2 ? n1 : n2;
  temp2 = n3 >= n4 ? n3 : n4;

	return temp1 >= temp2 ? temp1 : temp2;
}

bool create (const char *file, unsigned initial_size) {
  if (file == NULL) {
    exit(-1);
  }

  // locking 처리
  lock_acquire(&lock);
  bool success = filesys_create(file, initial_size);
  // unlocking 처리
  lock_release(&lock);
  return success;
}

bool remove (const char *file) {
  if (file == NULL) {
    exit(-1);
  }

  // locking 처리
  lock_acquire(&lock);
  bool success = filesys_remove(file);
  // unlocking 처리
  lock_release(&lock);

  return success;
}

int open (const char *file) {
	if (file == NULL) {
    exit(-1);
  }

	struct file *opened_file = filesys_open(file);
	if (opened_file == NULL) {
		return -1;
	}

  // WRITE 거부 처리 (열려는 파일이 동일 파일이 아닌 경우)
	if (!strcmp(thread_name(), file)) {
    file_deny_write(opened_file);
  }

  struct thread *thd = thread_current();
  int fd = thd->files_length;

  thd->files[fd] = opened_file;
  thd->files_length += 1;

	return fd;
}

void close (int fd) {
  // file 확인
  struct thread *thd = thread_current();
  struct file *file = thd->files[fd];
  if (file == NULL) {
    exit(-1);
  }

  // file close and initialization
  file_close(file);
	thd->files[fd] = NULL;
}

int filesize (int fd) {
  // file 확인
  struct file *file = thread_current()->files[fd];
  if (file == NULL) {
    exit(-1);
  }
  
  return file_length(thread_current()->files[fd]);
}

void seek (int fd, unsigned position) {
  // file 확인
  struct file *file = thread_current()->files[fd];
  if (file == NULL) {
    exit(-1);
  }
  
	file_seek(thread_current()->files[fd], position);
}

unsigned tell (int fd) {
  // file 확인
  struct file *file = thread_current()->files[fd];
  if (file == NULL) {
    exit(-1);
  }

	return file_tell(thread_current()->files[fd]);
}
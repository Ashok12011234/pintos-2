#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "process.h"

static void syscall_handler (struct intr_frame *);
static int write (int, const void *, unsigned);
extern bool running;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int * p = f->esp;

	uint32_t *esp;
    esp = f->esp;

  int system_call = * p;

	switch (system_call)
	{
		case SYS_HALT:
		shutdown_power_off();
		break;

		case SYS_EXIT:
		thread_current()->parent->ex = true;
		thread_exit();
		break;

		case SYS_WRITE:
		f->eax = write (*(esp + 1), (void *) *(esp + 2), *(esp + 3));
		break;

		default:
		printf("No match\n");
	}
}

int
write (int fd, const void *buffer, unsigned size)
{
  
  int status = 0;
   if (fd == STDOUT_FILENO)
     {
       putbuf (buffer, size);
       return size;
     }

}
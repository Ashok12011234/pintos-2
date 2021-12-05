#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);
static int write (int, const void *, unsigned);
void check_addr(const void*);
void close_all_files(struct list* files);
struct proc_file* list_search(struct list* files, int fd);
void close_file(struct list* files, int fd);
static int read (int, void *, unsigned);
int open(int fd);
void exit (int error_code);

struct proc_file {
	struct file *ptr;		//file pointer
	int fd;				//file descriptor	
	struct list_elem elem;
};


syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  

	uint32_t *esp;
    esp = f->esp;

	check_addr(esp);

	int fd = *((int*)f->esp + 1);
	void* buffer = (void*)(*((int*)f->esp + 2));
	unsigned size = *((unsigned*)f->esp + 3);

	switch (*esp)
	{
		case SYS_HALT:
		shutdown_power_off();
		break;

		case SYS_EXEC:
		check_addr(fd);
		f->eax =process_execute(fd);
		break;

		case SYS_CREATE:
		check_addr(fd);
		f->eax = filesys_create(fd,buffer);
		break;

		case SYS_REMOVE:
		check_addr(fd);
		bool status=filesys_remove(fd);
		f->eax = status;
		break;

		case SYS_OPEN:
		f->eax=open(fd);
		break;

		case SYS_FILESIZE:
		f->eax = file_length (list_search(&thread_current()->files, fd)->ptr);
		break;

		case SYS_READ:
		f->eax = read (fd,buffer, size);
		break;
		
		case SYS_WAIT:
		check_addr(fd);
		f->eax = process_wait(fd);
		break;

		case SYS_EXIT:
		exit(fd);
		break;

		case SYS_WRITE:
		f->eax = write (fd, buffer, size);
		break;

		case SYS_CLOSE:
		close_file(&thread_current()->files,fd);
		break;

		case SYS_SEEK:
		check_addr(fd);
		file_seek(list_search(&thread_current()->files, fd)->ptr,buffer);
		break;

		case SYS_TELL:
		check_addr(fd);
		f->eax = file_tell(list_search(&thread_current()->files, fd)->ptr);
		break;

		default:
		printf("No match\n");
	}
}

int 
open(int fd){

	int status=0;
	check_addr(fd);
		
	struct file* fptr = filesys_open (fd);
		
	if(fptr==NULL)
		status= -1;
	else
	{
		struct proc_file *pfile = malloc(sizeof(*pfile));
		pfile->ptr = fptr;
		pfile->fd = thread_current()->fd_count;
		thread_current()->fd_count++;
		list_push_back (&thread_current()->files, &pfile->elem);
		status= pfile->fd;
	}

	return status;

}

int
write (int fd, const void *buffer, unsigned size)
{
	check_addr(buffer);
	check_addr(buffer + size - 1);

	struct proc_file* fd_struct;
	int status = 0;

	if (fd == STDOUT_FILENO)
		{
		putbuf (buffer, size);
		return size;
		}
	if (fd == STDIN_FILENO)
	{
		return -1;
	}

	fd_struct = list_search(&thread_current()->files, fd);
	if (fd_struct != NULL)
	status = file_write (fd_struct->ptr, buffer, size);

	return status;
}

void close_all_files(struct list* files)
{

	struct list_elem *e;

      for (e = list_begin (files); e != list_end (files);
           e = list_next (e))
        {
          struct proc_file *f = list_entry (e, struct proc_file, elem);

	      	file_close(f->ptr);
	      	list_remove(e);
        }
} 

void exit (int error_code){
		thread_current()->parent->is_exited = true;
		thread_current()->exit_error = error_code;
		thread_exit();
}

void check_addr(const void *vaddr)
{
	
	if (!is_user_vaddr(vaddr))
	{
		exit(-1);	
	}
	void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
	if (!ptr)
	{
		exit(-1);
	}
}

struct proc_file* list_search(struct list* files, int fd)
{

	struct list_elem *e;

      for (e = list_begin (files); e != list_end (files);
           e = list_next (e))
        {
          struct proc_file *f = list_entry (e, struct proc_file, elem);
          if(f->fd == fd)
          	return f;
        }
   return NULL;
}

void close_file(struct list* files, int fd)
{

	struct list_elem *e;

      for (e = list_begin (files); e != list_end (files);
           e = list_next (e))
        {
          struct proc_file *f = list_entry (e, struct proc_file, elem);
          if(f->fd == fd)
          {
          	file_close(f->ptr);
          	list_remove(e);
          }
        }
}

int
read (int fd, void *buffer, unsigned size)

{
	struct proc_file* fd_struct  ; 
	int status = 0;

	check_addr(buffer);
	check_addr(buffer + size - 1);


  if (fd == STDOUT_FILENO)
    {
      return -1;
    }

  if (fd == STDIN_FILENO)
    {
      uint8_t c;
      unsigned counter = size;
      uint8_t *buf = buffer;
      while (counter > 1 && (c = input_getc()) != 0)
        {
          *buf = c;
          buffer++;
          counter--; 
        }
      *buf = 0;
      return (size - counter);
    } 

  fd_struct = list_search(&thread_current()->files, fd);
  if (fd_struct != NULL)
    status = file_read (fd_struct->ptr, buffer, size);

  return status;
}


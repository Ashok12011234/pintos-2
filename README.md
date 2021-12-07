# pintos-2

		     +--------------------------+
       	       	     |		CS 2042		|
		     | PROJECT 2: USER PROGRAMS	|
		     | 	   DESIGN DOCUMENT     	|
		     +--------------------------+

---- PRELIMINARIES ----

>> If you have any preliminary comments on your submission, notes for the
>> TAs, or extra credit, please give them here.

>> Please cite any offline or online sources you consulted while
>> preparing your submission, other than the Pintos documentation, course
>> text, lecture notes, and course staff.

https://static1.squarespace.com/static/5b18aa0955b02c1de94e4412/t/5b85fad2f950b7b16b7a2ed6/1535507195196/Pintos+Guide

			   ARGUMENT PASSING
			   ================

---- DATA STRUCTURES ----

>> A1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

none 

---- ALGORITHMS ----

>> A2: Briefly describe how you implemented argument parsing.  How do
>> you arrange for the elements of argv[] to be in the right order?
>> How do you avoid overflowing the stack page?

How to implement argument parsing?
----------------------------------
I did setup the stack inside setup_stack().Process_execute provides cmd_name, including file name and arguments
string. First, I separated the first token and the rest, which are file name and
arguments. I use file name as the new thread's name. And pass down the full command  
to start_process(), load() and setup_stack().

Way of arranging for the elements of argv[] to be in the right order.
--------------------------------------------------------------------
I scan through the argument string backwards, so that the first token is the last argument, the last token is the first argument. 
I kept decreasing esp pointer to setup the argv[] elements. 

How to avoid overflowing the stack page?
----------------------------------------
none

---- RATIONALE ----

>> A3: Why does Pintos implement strtok_r() but not strtok()?

save_ptr(placeholder) in strtok_r() is provided by the caller but not in strtok()

>> A4: In Pintos, the kernel separates commands into a executable name
>> and arguments.  In Unix-like systems, the shell does this
>> separation.  Identify at least two advantages of the Unix approach.

1.Shortening the time inside kernel
2.Robust checking.

			     SYSTEM CALLS
			     ============

---- DATA STRUCTURES ----

>> B1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

In thread.h

struct thread
  {
    ......
    
    bool is_exited;    //to check whether thread is exited or not

    struct thread* parent;	//parent of the thread
    int exit_error;		//to save error code
    struct list files;		//to save files of thread
    int fd_count;		//to save count of files to identify each file by unique fd

    ......
  };


struct proc_file {
	struct file *ptr;		//file that is opened
	int fd;				//unique file descriptor number	
	struct list_elem elem;
};

struct proc_file is used to keep track of files which are opened by process.

>> B2: Describe how file descriptors are associated with open files.
>> Are file descriptors unique within the entire OS or just within a
>> single process?

In my implementation, the file descriptor is unique just within a single process. I keep a file
list inside thread struct to store threads's file.I used fd_count to have a unique file descriptor for each file.

---- ALGORITHMS ----

>> B3: Describe your code for reading and writing user data from the
>> kernel.

Read:
First, check if the buffer and buffer + size are both valid pointers, if not,
exit(-1). After that check if fd is in the two special cases: STDOUT_FILENO
and STDIN_FILENO. If it is STDOUT_FILENO, then it is standard output, so return -1. 
If fd is STDIN_FILENO, then retrieve keys from standard input. After that,return 0. 
Otherwise, find the open file according to fd number from the current thread's files list. 
Then use file_read in filesys to read the file, get status and return the status.

Write:
Similar with read system call, first need to make sure the given buffer
pointer is valid.When the given fd is STDIN_FILENO, then return -1. 
When fd is STDOUT_FILENO, then use putbuf to print the content of buffer to the console. 
Other than these two cases, find the open file through fd number. Use file_write to write 
buffer to the file and get the status and return the status.

>> B4: Suppose a system call causes a full page (4,096 bytes) of data
>> to be copied from user space into the kernel.  What is the least
>> and the greatest possible number of inspections of the page table
>> (e.g. calls to pagedir_get_page()) that might result?  What about
>> for a system call that only copies 2 bytes of data?  Is there room
>> for improvement in these numbers, and how much?


The least number is 1. If the first inspection get a page head
back.

The greatest number might be 4096 if it’s not contiguous.

>> B5: Briefly describe your implementation of the "wait" system call
>> and how it interacts with process termination.

I just keep it as infinite loop.It checks whether process is exited or not.If process still alive it loop infinitely else it will end and return -1.

>> B6: Any access to user program memory at a user-specified address
>> can fail due to a bad pointer value.  Such accesses must cause the
>> process to be terminated.  System calls are fraught with such
>> accesses, e.g. a "write" system call requires reading the system
>> call number from the user stack, then each of the call's three
>> arguments, then an arbitrary amount of user memory, and any of
>> these can fail at any point.  This poses a design and
>> error-handling problem: how do you best avoid obscuring the primary
>> function of code in a morass of error-handling?  Furthermore, when
>> an error is detected, how do you ensure that all temporarily
>> allocated resources (locks, buffers, etc.) are freed?  In a few
>> paragraphs, describe the strategy or strategies you adopted for
>> managing these issues.  Give an example.

First, avoiding bad user memory access is done by checking before validating, by
checking I mean using the function check_addr I wrote to check whether it’s
NULL, whether it’s a valid user address and whether it’s been mapped in the
process’s page directory. Taking “write” system call as an example, the esp
pointer is checked first, if it's invalid, terminate the process. Then after enter into write function, the buffer
beginning pointer and the buffer ending pointer(buffer + size - 1) will be
checked before being used. 


---- SYNCHRONIZATION ----

>> B7: The "exec" system call returns -1 if loading the new executable
>> fails, so it cannot return before the new executable has completed
>> loading.  How does your code ensure this?  How is the load
>> success/failure status passed back to the thread that calls "exec"?

none

>> B8: Consider parent process P with child process C.  How do you
>> ensure proper synchronization and avoid race conditions when P
>> calls wait(C) before C exits?  After C exits?  How do you ensure
>> that all resources are freed in each case?  How about when P
>> terminates without waiting, before C exits?  After C exits?  Are
>> there any special cases?

none

---- RATIONALE ----

>> B9: Why did you choose to implement access to user memory from the
>> kernel in the way that you did?
I chose the first method is to verify
the validity of a user-provided pointer, then I dereference it
because this approach is simple and I cant understand second approach.
 
>> B10: What advantages or disadvantages can you see to your design
>> for file descriptors?

Advantages:
1) Thread-struct’s space is minimized
2) Kernel is aware of all the open files, which gains more flexibility to manipulate the opened files.

Disadvantages:
1) Consumes kernel space, user program may open lots of files to crash the
kernel.
2) The inherits of open files opened by a parent require extra effort to be
implement.

>> B11: The default tid_t to pid_t mapping is the identity mapping.
>> If you changed it, what advantages are there to your approach?

none
			   SURVEY QUESTIONS
			   ================

Answering these questions is optional, but it will help us improve the
course in future quarters.  Feel free to tell us anything you
want--these questions are just to spur your thoughts.  You may also
choose to respond anonymously in the course evaluations at the end of
the quarter.

>> In your opinion, was this assignment, or any one of the three problems
>> in it, too easy or too hard?  Did it take too long or too little time?

>> Did you find that working on a particular part of the assignment gave
>> you greater insight into some aspect of OS design?

>> Is there some particular fact or hint we should give students in
>> future quarters to help them solve the problems?  Conversely, did you
>> find any of our guidance to be misleading?

>> Do you have any suggestions for the TAs to more effectively assist
>> students, either for future quarters or the remaining projects?

>> Any other comments?


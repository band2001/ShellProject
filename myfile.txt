/* example of fork() system call.  RAB 1/03 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
  int ret;  /* for return value from fork() */

  printf("I am the PARENT.  My pid is %d\n", getpid());

  if ((ret = fork()) < 0) {
    printf("fork() FAILED");
    perror("forkeg");
    _exit(1);
  }
  /* fork() succeeded */

  if (ret != 0) {
    int ret_wait, wait_status;
    if((ret_wait = wait(&wait_status))<0){
        perror("wait()");
    }
    printf("I am the PARENT.  My pid is %d, and fork() returned %d.\n",
	   getpid(), ret);
    printf("For PARENT, getppid() returns %d\n", getppid());
    /* NOTE: parent should ordinarily wait(), to collect child's exit status */
  } else {
    printf("I am the CHILD.  My pid is %d, and fork() returned %d.\n",
	   getpid(), ret);
    printf("For CHILD, getppid() returns %d\n", getppid());
    _exit(1);
  }

  /* only parent sees this code, because child exited */

  printf("I am the PARENT.  My pid is %d, and I'm signing off.\n", getpid());

  _exit(0); 
}

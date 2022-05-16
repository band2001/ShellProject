#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include<time.h>

#define NORMAL 0
#define NEWLINE 1
#define NULLBYTE 2

int execute(char* pathname, char** token, char** envp);

int time_command(char** tok, char** envp){ // function to calculate time in seconds taken to execute a command
/* PARAMETERS: tok -- array of tokens
                envp -- envp from main arguments 
    RETURN: 0 on success, -1 on failure
    STATE CHANGE: prints out the time taken            */
    
  int i = 0; // increment used for the for loop
  for(i; i<8; i++){ // eliminates time from the tok array
    tok[i] = tok[i+1]; // shifts all elements left by 1
  }
  
  int time_retA; // will store time since epoch before execute is called
  if((time_retA = time(NULL))<0){ // calls time()
    perror("time()"); // time() error handling
    return -1;
  }
  int execute_ret;
  if((execute_ret = execute(tok[0],tok,envp))<0){ // executes the command
    printf("execute() failed, please try again\n"); // error handling for execute()
    return -1;
  }
  int time_retB; // will store time since epoch after execute is called
  if((time_retB = time(NULL))<0){ // calls time()
    perror("time()"); // time() error handling
    return -1;
  }

  int time_used = time_retB - time_retA; // time taken for the command
 
  printf("Time taken for execution: %d secs\n",time_used); // prints the time taken for the command

  return 0;
  
}

int cd(char** tok, char** envp){ // implement cd command
  /* PARAMETERS: tok: char** the array with the tokens entered from standard input 
                 envp: char**, the envp from main
     RETURN: 0 on success, -1 on failure
  */
  int chdir_ret; // return from chdir
  if((chdir_ret = chdir(tok[1]))<0) { // calls chdir with the given path which is tok[1]
    perror("chdir()"); // chdir error handling
    return -1;
  }
  return 0; // returns 0 on completion
}

int pipeline(char**token, char** envp, int index){ // implement pipeline
  /* PARAMETERS: token: char** array with the tokens entered from standard input
                 envp: char**, the envp from main
                 index: int, the index where the '|' character is located in token
     RETURN: 0 on success, -1 on failure. 
  */

  int stdin_copy = dup(0); // creates a copy of standard input fd for reopening
  int stdout_copy = dup(1); // creates a copy of standard output fd for reopening

  int pipearr[2]; // two integer array to pass to the pipe

  char** tok1 = (char**) malloc(sizeof(char*)*(index+1)); // dynamically allocating space for tokens before '|'
  char** tok2 = (char**) malloc(sizeof(char**)*(10-(index+1))); // "  "  " after '|'

  int ii = 0;
  while(*token[ii] != '|' && token[ii] != NULL){ // creating dynamic memory for each C-Style string where each token can be up to 20 characters
      tok1[ii] = (char*) malloc(sizeof(char)*20);
      tok1[ii] = token[ii]; // copying values from token to tok1
      ii++; // increment ii
  }
  tok1[ii] = 0; // adds nullbyte to end of tok1
  ii = index+1;
  while(token[ii]!= NULL){ // creating dynamic memory for each C-Style string where each token can be up to 20 characters
      tok2[ii-(index+1)] = (char*) malloc(sizeof(char*)*20);
      tok2[ii-(index+1)] = token[ii]; // copying values from token to tok2
      ii++;
  }
  tok2[ii-(index+1)] = NULL;

  int pipe_ret; // return value from pipe
  if((pipe_ret = pipe(pipearr)) < 0){ // calling pipe
    perror("pipe()");
    return -1;
  }

  int ret_fork; // return from fork
  if ((ret_fork = fork()) < 0) { // calling fork() with correct error handling
    printf("fork() FAILED");
    perror("forkeg");
    _exit(1);
  }

  // FIRST FORK; PIPELINE BEGINS IN SECOND FORK

  if(ret_fork != 0){ // this is the ultimate parent process, waits for child process to finish before proceeding.
    int ret_wait, wait_status; // the return from wait, the argument to pass to wait
    if((ret_wait = waitpid(-1, &wait_status, P_PID))<0){
        perror("waitpid()");
    }
  
    // END OF ALL FORKING

  } else { // CHILD PROCESS OF FIRST FORK
      
      int ret_fork1; // return from fork
      if ((ret_fork1 = fork()) < 0) { // calling fork() with correct error handling
        printf("fork() FAILED");
        perror("forkeg");
        _exit(1);
      }

      // PIPELINE CONTINUES IN ELSE STATEMENT

      if(ret_fork1 != 0){
        // PIPELINE CONTINUES; PARENT PROCESS OF SECOND FORK
        int ret_wait1, wait_status1; // the return from wait, the argument to pass to wait
        if((ret_wait1 = waitpid(-1,&wait_status1,P_PID))<0){
            perror("waitpid()");
        }

        close(0); // closes standard input
        close(pipearr[1]); // closing the write part of the pipeline

        int dup_ret;
        if((dup_ret = dup(pipearr[0]))<0){ // redirecting standard input to be pipearr[0]
            perror("dup()");
            return -1;
        }
        int ret_ex; // return from execve
        if((ret_ex = execve(tok2[0],tok2,envp))<0){ // calls execve with using tok1 for arguments; prints error and returns
          perror("pipeline right: execve()");                           // -1 on failure
          return -1;
        }
        
        // END OF PIPELINE

      } else {

        // CHILD OF SECOND FORK
        // BEGINNING OF PIPELINE

        close(1); // closing standard output
        close(pipearr[0]); // closing the read part of the pipeline

        int dup_ret;
        if((dup_ret = dup(pipearr[1]))<0){ // redirecting standard output to pipearr[1]
            perror("dup()");
            return -1;
        }

        int ret_ex; // return from execve
        if((ret_ex = execve(tok1[0],tok1,envp))<0){ // calls execve with using tok1 for arguments; prints error and returns
          perror("pipeline left: execve()");                           // -1 on failure
          return -1;
        }
        _exit(1); // exiting child process -- go to IF STATEMENT FOR PARENT PROCESS
      }
      _exit(1); // END OF CHILD OF FIRST FORK, RETURN TO PARENT OF FIRST FORK
  }
  ii = 0; // next lines are for freeing dynamically allocated memory
  while(tok2[ii]!=NULL){
      free(tok2[ii]); ii++;
  }
  ii = 0;
  for(ii; ii<index; ii++){
      free(tok1[ii]);
  }
  free(tok2);
  free(tok1);

  close(pipearr[0]); // closes the open fd for pipearr[0]
  close(pipearr[1]); // closes the open fd for pipearr[0]
  
  int dup2_ret;
  if((dup2_ret=dup2(stdin_copy,0))<0) { //reopens standard input
    perror("dup2()");
    return -1;
  }
  close(stdin_copy); // closes the duplicate for std input
  dup2_ret=0;
  if((dup2_ret = dup2(stdout_copy,1))<0){ // reopenes standard output
    perror("dup2()");
    return -1;
  }
  close(stdout_copy); // closes the duplicate for std output

  return 0; // returns 0 on success

}

int execute(char* pathname, char** token, char** envp) { // executes commands entered in the shell
    /* Parameters: char* pathname - address to the executable
                   char** token - a char array of C-Style strings with the parsed words
                   char** envp - envp from main
       Return: int 0 on success, -1 on failure */
  
  if(strcmp(token[0], "time")==0) { // if time is entered in standard input, the time function is used instead.
      time_command(token, envp);
      return 0;
  }
  
  if(strcmp(token[0], "cd")==0) { // if cd is entered in standard input, the cd function is used instead.
      cd(token, envp);
      return 0;
  }
  int i = 0;
  for(i; i<10; i++){ // checks for a pipe in the command
    if(token[i] != NULL){
      if(*token[i]=='|'){
        pipeline(token, envp, i);
        return 0;
      }
    }
  }

  int ret_fork; // return from fork
    if ((ret_fork = fork()) < 0) { // calling fork() with correct error handling
      printf("fork() FAILED");
      perror("forkeg");
      _exit(1);
    }

    if(ret_fork != 0){ // this is the parent process, waits for child process to finish before proceeding.
      int ret_wait, wait_status; // the return from wait, the argument to pass to wait
      if((ret_wait = wait(&wait_status))<0){
          perror("wait()");
      }

    } else { // this is the child process; executes the command at the given pathfile
      int ret_ex; // return from execve
      if((ret_ex = execve(pathname,token,envp))<0){ // calls execve with using token for arguments; prints error and returns
        perror("execve()");                           // -1 on failure
        return -1;
      }
      _exit(1); // exiting child process
  }
  return 0;
}

char** get_token(char** ptr, int* status){
    /* Parameters: char** ptr - a pointer to an array holding a c-style string of standard input
                   int* status - a pointer with the memory address of a variable holding the status of the get_token function
       Return: a char array of C-Style strings with the parsed words */
  
  int SIZE = 0; // size of ptr (number of chars, including whitespace)
  int COUNT = 0; // word count; also used to ensure only five tokens passed

  char **tok = (char**) malloc(sizeof(char*)*10); // Creating dynamic memory for an array of C-Style strings with 10 spots
  int ii = 0; // incrementer for for loop
  for(ii; ii<10; ii++){ // creating dynamic memory for each C-Style string where each token can be up to 20 characters
      tok[ii] = (char*) malloc(sizeof(char)*40);
  }

  char **tok_reset = tok; // creating a duplicate pointer pointing to the same memory address as tok to use to free the dynamic memory
  char* ptr_reset = *ptr; // creating a duplicate pointer pointing to the same memory address as ptr to free dynamic memory
  int my_bool = 1; // a value used as the condition for the following while loop

  while(my_bool){ // while loop to loop through the ptr array

    if(COUNT>9){ // makes sure tok only contains up to five tokens
        *status = 3;
        printf("ERROR: Too many tokens entered. Enter up to nine tokens.\n"); // Error printed using standard output
        my_bool = 0; // exiting the while loop
        break;
    }
    
    if(**ptr == 10){ // checks for newline characters, exits the loop when one is detected; if it is the first char, it sets status=1
        if (!COUNT){*status = 1;}
        my_bool = 0; // exiting the loop
        break;
    }
    while(isspace(**ptr)){ // parses through all "white" space
      if(**ptr == 10){ // again checks for a newline in the middle of the white space (same as situation above)
        if (!COUNT){*status = 1;}
        my_bool = 0;
        break;
      }
      (*ptr)++; // increments the memory address ptr is point to
      SIZE++; // increments SIZE
    }
    if(**ptr == 0){ // checking for nullbyte
      if (!COUNT){*status = 2;} // if a nullbyte is the first character, status=2
      my_bool = 0; // exits the loop
      break;
    }
    int size_1 = 0; // used to temporarily store the size of each word/command (touching characters without whitespace)
    while(!isspace(**ptr)){ // running a while loop while there is no white space
      size_1++; (*ptr)++; // incrementing size and the memory address of ptr
    }
    if(size_1>=39){ // checks to make sure the token is not too long
        printf("ERROR: Token contains too many characters. A single token can have up to 39 characters.\n");
        *status = 4;
        my_bool = 0;
        break;
    }

    (*ptr)-= size_1; // un-incrementing the pointer back to the start of a continuous set of characters
    
    strncpy(tok[COUNT],*ptr,size_1); // copying the continuous set of characters to tok[COUNT]

    (*ptr) += size_1; // incrementing pointer past set of continuous characters just added to tok
    SIZE += size_1; // adding to the total size of ptr
    COUNT += 1; // increasing the word count

    *status = 0; // status=0 -- success
  }

  if(*status == 1 || *status == 2){ // if status=1 or status=2, returns NULL
      return NULL;
  }

  if(COUNT<=9){tok[COUNT] = (char*) 0;} // adds a nullbyte after the last token added

  tok=tok_reset; // reseting the tok pointer to free memory correctly
  *ptr=ptr_reset; // reseting the ptr to free memory correctly
  return tok; // returning the array with continuous words, separated by a nullbyte

}

int main(int argc, char **argv, char**envp) {
  char **token; // will contain the array returned from get_token
  char *line = (char*) malloc(80); // creates a dynamically allocated array to enter a line of input
  size_t size = 80; // size of the dynamically allocated array

  while(1) {
    int ret; // return from getline
    if ((ret = getline(&line, &size, stdin)) < 0) { // calls getline, returns -1 if error
      perror("getline");
      return -1;
    }
    if(ret==EOF){ // checks to see if end of file has been reached; if it has, the loop is broken.
      break;
    }

    int status; // will contain the status of get_token
    token = get_token(&line, &status); // calls get_token
    if (token == NULL){ // if get_token returns null, then an error is printed and -1 returned
      printf("get_token() returned NULL\n");
      continue;
    }
    if (status > 0){ // if status is not 0, offers new opportunity to enter a command
      continue;
    } 
    int execute_ret;
    if((execute_ret = execute(token[0],token, envp))<0){ // executes the commands entered in the shell, calling execute
      printf("execute() failed, please try again\n"); // error handling for execute()
      continue;
    }
  }
  
  free(line); // the next 6 lines frees all dynamically allocated memory
  int ii = 0;
  for(ii; ii<10; ii++){
      free(token[ii]);
  }
  free(token);

  _exit(0); // ends parent process
}
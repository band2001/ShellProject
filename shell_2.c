#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>

#define NORMAL 0
#define NEWLINE 1
#define NULLBYTE 2

char** get_token(char** ptr, int* status){
    /* Parameters: char** ptr - a pointer to an array holding a c-style string of standard input
                   int* status - a pointer with the memory address of a variable holding the status of the get_token function
       Return: a char array of C-Style strings with the parsed words */
  
  int SIZE = 0; // size of ptr (number of chars, including whitespace)
  int COUNT = 0; // word count; also used to ensure only five tokens passed

  char **tok = (char**) malloc(sizeof(char*)*5); // Creating dynamic memory for an array of C-Style strings with 5 spots
  int ii = 0; // incrementer for for loop
  for(ii; ii<5; ii++){ // creating dynamic memory for each C-Style string where each token can be up to 20 characters
      tok[ii] = (char*) malloc(sizeof(char)*20);
  }

  char **tok_reset = tok; // creating a duplicate pointer pointing to the same memory address as tok to use to free the dynamic memory
  char* ptr_reset = *ptr; // creating a duplicate pointer pointing to the same memory address as ptr to free dynamic memory
  int my_bool = 1; // a value used as the condition for the following while loop

  while(my_bool){ // while loop to loop through the ptr array
    

    if(COUNT>4){ // makes sure tok only contains up to five tokens
        *status = 3;
        printf("ERROR: Too many tokens entered. Enter up to four tokens.\n"); // Error printed using standard output
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
    if(size_1>=19){ // checks to make sure the token is not too long
        printf("ERROR: Token contains too many characters. A single token can have up to 19 characters.\n");
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

  if(COUNT<=4){tok[COUNT] = (char*) 0;} // adds a nullbyte after the last token added

  tok=tok_reset; // reseting the tok pointer to free memory correctly
  *ptr=ptr_reset; // reseting the ptr to free memory correctly
  return tok; // returning the array with continuous words, separated by a nullbyte

}

int main(int argc, char **argv, char**envp) {
  char *line = (char*) malloc(80); // creates a dynamically allocated array to enter a line of input
  size_t size = 80; // size of the dynamically allocated array
  int ret; // return from getline
  printf("Enter a line of input\n"); // prompt to enter a line of input
  if ((ret = getline(&line, &size, stdin)) < 0) { // calls getline, returns -1 if error
    perror("getline");
    return -1;
  } 
  
  char **token; // will contain the array returned from get_token 
  int status; // will contain the status of get_token
  token = get_token(&line, &status); // calls get_token
  if (token == NULL){ // if get_token returns null, then an error is printed and -1 returned
    printf("get_token() returned NULL\n");
    return -1;
  }

  int ret_ex; // return from execve
  if((ret_ex = execve(token[0],token,envp))<0){ // calls execve with using token for arguments; prints error and returns
    perror("execve()");                           // -1 on failure
    return -1;
  }
  
  free(line); // the next 6 lines frees all dynamically allocated memory
  int ii = 0;
  for(ii; ii<5; ii++){
      free(token[ii]);
  }
  free(token);

  return 0; // returns 0 upon success
}
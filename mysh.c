

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// This is the maximum number of arguments your shell should handle for one command
#define MAX_ARGS 128

//split string function modified to feed split strings into an array
void split_string(char* str, char** arr, char* delims) {
  char* current_position = str;
  int k = 0;
  while(true) {
    // Call strpbrk to find the next occurrence of a delimeter
    char* delim_position = strpbrk(current_position, ";&");

    if(delim_position == NULL) {
      // There were no more delimeters.
        arr[k] = current_position;
        arr[k+1] = NULL;
      
      return ;

    } else {
      // There was a delimeter. First, save it.
      char delim = *delim_position;
      delims[k] = delim;
      // Overwrite the delimeter with a null terminator so we can print just this fragment
      *delim_position = '\0';
        arr[k] = current_position;
      
      k++;
    }

    // Move our current position in the string to one character past the delimeter
    current_position = delim_position + 1;
  }
}//split string end


int main(int argc, char** argv) {

  // If there was a command line option passed in, use that file instead of stdin
  if (argc == 2) {
    // Try to open the file
    int new_input = open(argv[1], O_RDONLY);
    if (new_input == -1) {
      fprintf(stderr, "Failed to open input file %s\n", argv[1]);
      exit(1);
    }

    // Now swap this file in and use it as stdin
    if (dup2(new_input, STDIN_FILENO) == -1) {
      fprintf(stderr, "Failed to set new file as input\n");
      exit(2);
    }
  }

  char* line = NULL;     // Pointer that will hold the line we read in
  size_t line_size = 0;  // The number of bytes available in line

  

  // Loop forever
  while (true) {



    // Print the shell prompt
    printf("$ ");

    // Get a line of stdin, storing the string pointer in line
    if (getline(&line, &line_size, stdin) == -1) {
      if (errno == EINVAL) {
        perror("Unable to read command line");
        exit(2);
      } else {
        // Must have been end of file (ctrl+D)
        printf("\nShutting down...\n");

        // Exit the infinite loop
        break;
      }
    }

//once command is read check for semicolons and split the string
char* argsin[MAX_ARGS];
char*input = line;
char*writable = strdup(input);
char delims[128];
split_string(writable,argsin,delims);


    // Execute the command instead of printing it below
int l = 0;
int delimiter = 0;
int exec_input = 0;
pid_t child_id;
int status;
//make space for a NULL character too
char* current[MAX_ARGS + 1];


//while there are still more commands eg. in case of & and ; keep going
while(argsin[l] != NULL) 
{
//Part A  
int i = 0;
int j =0;



while( (current[i] = strsep(&argsin[l]," \n\t")) != NULL && j < MAX_ARGS)
{
  if(strcmp(current[i],"")!=0)
     {
       //only add an element in the i indx if it is a valid element
       i++;   
     }
     else
     {
       //we need to break out of the loop if there is just empty space
       j++;
     }
}

current[i+1] = NULL;

//if encounter empty line prompt again
if(current[0] == NULL)
{

}
//exit mechanism
else if(strcmp(current[0],"exit") == 0)
{
 return 0;
}
//changing directory mechanism
else if (strcmp(current[0],"cd")== 0)
{
 
  //if nothing entered after cd, do not give error, only throw error if a path has been entered and it is incorrect.
  if(current[1] != NULL)
  {
  if (chdir(current[1]) == -1)
  {
    perror("No such file or directory\n");
  }
  }
 
}
else{
  
 child_id = fork();
  if (child_id == 0) {
//calling execvp in child process
   execvp(current[exec_input],&current[exec_input]);

    perror("exec failed");

    exit(57);

  } else if (child_id > 0) {
  
//if there is an & then we need to run function in background otherwise proceed normally and wait for child process
if(delims[delimiter] != '&' )
   { 
    pid_t rc = wait(&status);
    
    if (rc == -1) {
      perror("wait failed");
      exit(2);
    }

    printf("[%s exited with status %d]\n",current[exec_input],WEXITSTATUS(status));
   }
   


  } else {
    perror("fork failed");
  }
  
}
//move to the next command and next delimiter
l++;
delimiter++;

}

//check if background commands have exited
pid_t id = waitpid(-1, &status, WNOHANG);
while ( id > 0)
{
    printf("[backgroud command with id: %d  exited with status %d]\n",id,WEXITSTATUS(status));
    id = waitpid(-1, &status, WNOHANG);

}


}

  // If we read in at least one line, free this space
  if (line != NULL) {
    free(line);
  }
  

  return 0;
}


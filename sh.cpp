#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <libgen.h>

#define MAX_BUFFER 1024     // max line buffer
#define MAX_ARGS 20         // max # args
using namespace std;

void env(char **e);
void io(char **args);
int checkAmp(char **args);

extern char **environ; //env variables

pid_t pid;             // process ID
int status;             // status for fork/exec process
int in, out, a, b, c; // I/O redirection parameters
char *inputFile, *outputFile; // I/O input and output files

// get the environment variables
void env(char **e){
  FILE *fd;
  char **env = e;

  // IO redirection
  if (b == 1){
    fd = fopen(outputFile, "w");
  }
  else if (c == 1){
    fd = fopen(outputFile, "a");
  }

  //if ouput or append then fprintf
  if (b == 1 || c == 1){
    while(*env){
      fprintf(fd,"%s\n", *env++);
    }
    fclose(fd);
  }
  //otherwise just print to screen
  else{
    while(*env){
      printf("%s\n", *env++);
      
    }
  }  
}

// check the command for any I/O redirection
void io(char **args){
  // reset input and output and append
  a = 0;
  b = 0;
  c = 0;

  int i = 0;

  while(args[i] != NULL){
    if (!strcmp(args[i], "<")){           //check for input <
      strcpy(args[i], "\0");
      inputFile = args[i+1];
      a = 1;
    }
    else if (!strcmp(args[i], ">")){      //check for output >
      outputFile = args[i+1];
      args[i] = NULL;
      b = 1;
      break;
    }
    else if (!strcmp(args[i], ">>")){     //check for append output >>
      outputFile = args[i+1];
      args[i] = NULL;
      c = 1;
      break;
    }
    i++;
  }
}

int checkAmp(char **args){
  int i = 0;
  int found = 0;
  while(args[i] != NULL){
    if (!strcmp(args[i], "&")){
      found = 1;
      args[i] = NULL; //remove the & and set to NULL so that the commmand will work
    }
    i++;
  }
  return found;
}

// the main function 
int main(int argc, char ** argv){
  char buf[MAX_BUFFER];
  char * args[MAX_ARGS];
  char ** arg;
  const char * path;
  char r[PATH_MAX];
  ssize_t c = readlink( "/proc/self/exe", r, PATH_MAX );
  if (c != -1) path = dirname(r);
  const char * prompt = path;
  int found = 0;
  int status;

  //check access first
  if(argc > 1) {
    freopen(argv[1], "r", stdin);
  }

  while(!feof(stdin)){
    cout << "1730sh:" << path << "$ ";

    if(fgets(buf, MAX_BUFFER, stdin)){
      arg = args;
      *arg++ = strtok(buf," \t\n");

      while ((*arg++ = strtok(NULL, " \t\n")));

      io(args); //check i/o redirections
      found = checkAmp(args); // check to see &

      if (args[0]) {
        // if there was an input redirection (<) 
        if (a == 1){
          if(!access(inputFile, R_OK)){ //check access
            freopen(inputFile, "r", stdin); // replace the stdin with the file
          }//if access
        }//if input = 1
	
	if(!strcmp(args[0], "export")){
	  char * var=args[1];
	  char *value=getenv(var);
	  char *string;
	  value=argv[2];
	  string = (char*)malloc(strlen(var));
	  if(!string)
            {
	      fprintf(stderr,"memory error");
	      exit(1);
            }   
	  strcpy(string,var);
	  strcat(string," ");
	  printf(" ",string);
	  if(putenv(string)!=0){
	      fprintf(stderr,"putenv fail ");
	      free(string);
	      exit(1);
            }
	  value = getenv(var);
	  continue;
	}

        //get the environment variables of the shell
        if (!strcmp(args[0], "env")) {
          env(environ); 
          continue;
        }//if environ
	
	if (!strcmp(args[0],"echo")) { 
          pid = getpid(); // get process id

          if((pid = fork()) == -1){
	    perror("FORK ERROR");
	    abort();
	  }else if(pid == 0){
	    setenv("parent", getenv("shell"), 1); //set parent
	    
	    //i/o redirection for output files
	    if(b == 1)
	      freopen(outputFile, "w", stdout);
	    else if(a == 1)
	      freopen(outputFile, "a+", stdout);
	    
	    if(execvp (args[0], args) == -1){
	      perror("EXEC CHILD ERROR");
	      abort();
	    }  //execute in the child thread
	  }else{                
	    if (!found) //determine background execution wait (&)
	      waitpid(pid, &status, WUNTRACED);
	  }
          continue;
	}
	
        if (!strcmp(args[0],"exit")) { 
          break; //break the loop so the program returns and ends
        }

	else{
	  pid = getpid();   
          if((pid = fork ())== -1) { 
	    perror("FORK ERROR");
	    abort();
	  }else if(pid == 0){
	    setenv("parent", getenv("shell"), 1); //set parent
	    if(b == 1)
	      freopen(outputFile, "w", stdout);
	    else if(c == 1)
	      freopen(outputFile, "a+", stdout); 

	    if(execvp (args[0], args) == -1){
	      perror("EXEC CHILD ERROR");
	      abort();
	    } //execute in child thread
	  }else{                
	    if (!found) //determine background execution wait (&)
	      waitpid(pid, &status, WUNTRACED);
	  }	  
	}
          continue;
      }//if args[0]
    }//if fgets

  }//while feof


  return 0;
}//main

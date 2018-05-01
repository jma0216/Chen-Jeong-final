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
#define MAX_ARGS 64         // max # args
#define SEPARATORS " \t\n"  // token sparators
using namespace std;

void cd(char *d);
void env(char **e);
void syserr(char * msg);
void checkIO(char **args);
int checkBackground(char **args);
void setMyShellEnv();

extern char **environ; //env variables
extern int errno;      // system error number
pid_t pid;             // process ID
int status;             // status for fork/exec process
int in, out, input, output, append; // I/O redirection parameters
char *inputFile, *outputFile; // I/O input and output files
FILE *fp;             //pointer to file for ouput file  

// get the environment variables
void env(char **e){
  char **env = e;
  // IO redirection
  if (output == 1){
    fp = fopen(outputFile, "w");
  }
  else if (append == 1){
    fp = fopen(outputFile, "a");
  }

  //if ouput or append then fprintf
  if (output == 1 || append == 1){
    while(*env){
      fprintf(fp,"%s\n", *env++);
    }
    fclose(fp);
  }
  //otherwise just print to screen
  else{
    while(*env){
      printf("%s\n", *env++);
    }
  }  
}


void syserr(char * msg){
  fprintf(stderr, "%s: %s\n", strerror(errno), msg);
  abort();
}

// check the command for any I/O redirection
void checkIO(char **args){
  // reset input and output and append
  input = 0;
  output = 0;
  append = 0;

  int i = 0;

  while(args[i] != NULL){
    if (!strcmp(args[i], "<")){           //check for input <
      strcpy(args[i], "\0");
      inputFile = args[i+1];
      input = 1;
    }
    else if (!strcmp(args[i], ">")){      //check for output >
      outputFile = args[i+1];
      args[i] = NULL;
      output = 1;
      break;
    }
    else if (!strcmp(args[i], ">>")){     //check for append output >>
      outputFile = args[i+1];
      args[i] = NULL;
      append = 1;
      break;
    }
    i++;
  }
}

//find the last non whitespace character and check if that is an ampersand
//if there is an ampersand then make dont_wait = 1
int checkBackground(char **args){
  int i =0;
  int dont_wait=0;
  while(args[i] != NULL){
    if (!strcmp(args[i], "&")){
      dont_wait = 1;
      args[i] = NULL; //remove the & and set to NULL so that the commmand will work
    }
    i++;
  }
  return dont_wait;
}

void setMyShellEnv(){
  char home_path[1024];
  getcwd(home_path, 1024);
  strcat(home_path, "/myshell");
  setenv("shell", home_path, 1);
}

void sigint_handler(int signum){
  //give warning that the signal has been disabled
  // wait for ENTER to be pressed before returning to the command line.
  printf("\nCTRL+C Interrupt Signal has been disabled. To exit use the 'quit' command. Press Enter to continue...\n");
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
  int dont_wait = 0;
  int status;

  signal(SIGINT, sigint_handler); // catches the CTRL C signal and calls an interrupt handler


  setMyShellEnv(); // get the shell environment

  //check access first
  if(argc > 1) {
    freopen(argv[1], "r", stdin);
  }

  while(!feof(stdin)){
    cout << "1730sh:" << path << "$ ";
    // fputs(prompt, stdout);
    if(fgets(buf, MAX_BUFFER, stdin)){
      arg = args;
      *arg++ = strtok(buf,SEPARATORS);

      while ((*arg++ = strtok(NULL,SEPARATORS)));

      checkIO(args); //check i/o redirections
      dont_wait = checkBackground(args); // check for background execution (that is &)

      if (args[0]) {
        // if there was an input redirection (<) 
        if (input == 1){
          if(!access(inputFile, R_OK)){ //check access
            freopen(inputFile, "r", stdin); // replace the stdin with the file
          }//if access
        }//if input = 1

        //get the environment variables of the shell
        if (!strcmp(args[0], "environ")) {
          env(environ); //call helper
          continue;
        }//if environ

	if (!strcmp(args[0],"echo")) { 
          pid = getpid(); // get process id
          if((pid = fork()) == -1){
	    syserr((char*) "fork error");
	  }else if(pid == 0){
	    setenv("parent", getenv("shell"), 1); //set parent
	    
	    //i/o redirection for output files
	    if(output == 1)
	      freopen(outputFile, "w", stdout);
	    else if(append == 1)
	      freopen(outputFile, "a+", stdout);
	    
	    execvp (args[0], args);  //execute in the child thread
	    syserr((char*)"execvp error");
	  }else{                
	    if (!dont_wait) //determine background execution wait (&)
	      waitpid(pid, &status, WUNTRACED);
	  }
          continue;
	}
	
        if (!strcmp(args[0],"quit")) { 
          break; //break the loop so the program returns and ends
        }

	else{
          pid = getpid();   
          switch (pid = fork ()) { 
	  case -1:
	    syserr((char*)"fork error");
	  case 0:
	    setenv("parent", getenv("shell"), 1); //set parent
	    //i/o redirection
	    if(output == 1)
	      freopen(outputFile, "w", stdout);
	    else if(append == 1)
	      freopen(outputFile, "a+", stdout); 

	    execvp (args[0], args); //execute in child thread
	    syserr((char*)"execvp error");
	  default:                
	    if (!dont_wait) //determine background execution wait (&)
	      waitpid(pid, &status, WUNTRACED);
	  }
          continue;
        }
      }

    }

  }
  return 0;
}

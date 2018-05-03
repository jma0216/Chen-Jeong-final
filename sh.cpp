#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <libgen.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

void env(char **e);
void io(char **args);
int checkAmp(char **args);

extern char **environ; 
int a;
int b;
int c;
pid_t pid;            
int in;
int out;
char *inFile, *outFile; 
int status;

/*
 * @param char ** args- pointer to a pointer to a char
 *
 * This function goes through argv and checks whether there is a 
 * I/O redirection and if there is which one.
 */
void io(char ** argv){
  // reset input and output and append
  a = 0;
  b = 0;
  c = 0;
  int i = 0;
  while(argv[i] != NULL){
    if (!strcmp(argv[i], "<")){           //input redirection
      strcpy(argv[i], "\0");
      inFile = argv[i+1];
      a = 1;
    }
    else if(!strcmp(argv[i], ">")){      //output redirection
      outFile = argv[i+1];
      argv[i] = NULL;
      b = 1;
      break;
    }
    else if(!strcmp(argv[i], ">>")){     //output redirection with appending
      outFile = argv[i+1];
      argv[i] = NULL;
      c = 1;
      break;
    } 
    i++;
  }
}

/*
 * @param argv
 *
 * Makes sure that the end of argv ends with NULL
 *
 */
int checkAmp(char **argv){
  int i = 0;
  int found = 0;
  while(argv[i] != NULL){
  if (!strcmp(argv[i], "&")){
    found = 1;
    argv[i] = NULL; //Replace & with NULL
  }
  i++;
  }
  return found;
}
/*
 * @param char **e
 * 
 * Gets the environmental variables and prints it on the screen
 *
 */
void listEnv(char **environ){
  FILE *fd;
  char **env = environ;
  
  if (b == 1){
    fd = fopen(outFile, "w");
  }
  else if (c == 1){
    fd = fopen(outFile, "a");
  }
  
  if (b == 1 || c == 1){
    while(*env){
      fprintf(fd,"%s\n", *env++);
    }
    fclose(fd);
  }
  else{
    while(*env){
      printf("%s\n", *env++);
    }
  }  
}

// the main function 
int main(int argc, char ** argv){
  
  char buf[1024];
  char * args[20];
  char ** arg;
  int found = 0;
  int status;
  char max[PATH_MAX]; 
  ssize_t count = readlink("/proc/self/exe", max, PATH_MAX);
  const char *path;
  if (count != -1) {
    path = dirname(max);
  }
 
  while(!feof(stdin)){
    cout << "1730sh" << path << "$ ";
     if(fgets(buf, 1024, stdin)){
      arg = args;
      *arg++ = strtok(buf," \t\n");
      while ((*arg++ = strtok(NULL, " \t\n")));

      io(args); //I/O Redirection
      found = checkAmp(args); //Make sure last space is null

      if (args[0]) {
        // Input redirection
        if (a == 1){
            freopen(inFile, "r", stdin); // Redirect stdin with file
        }//if input=1
		
	if(!strcmp(args[0], "export")){ //Add and change environment variables using export
	  char * v = args[1];
	  char *string;
	
	  string = (char*)malloc(strlen(v));  //allocates space
	  if(!string){
	    cout << stderr << "memory error";
	      exit(1);
            }
	  
	  strcpy(string,v);
	  strcat(string," ");
	  printf("Setting env variable.. %s \n",string);
	  
	  if(putenv(string)!= 0){         //error in setting variable
	    cout << stderr << endl;
	    free(string);
	    exit(1);
	  }
	  continue;
	}
	
        //get the environment variables of the shell
        if (!strcmp(args[0], "env")) {
          listEnv(environ); 
          continue;
        }//if environ
	
	if (!strcmp(args[0],"echo")) { 
          pid = getpid(); // get process id
	  
          if((pid = fork()) == -1){
	    perror("fork error");
	    abort();
	  }
	  else if(pid == 0){
	    setenv("parent", getenv("shell"), 1); 
	    
	    //i/o redirection for output
	    if(b == 1)
	      freopen(outFile, "w", stdout);
	    
	    //i/o redirection for input
	    else if(a == 1)
 	      freopen(outFile, "a+", stdout);
 	    
	    if(execvp (args[0], args) == -1){
	      perror("Execution child error");
	      abort();
	    }
	  } 
	  else{                
	    if (!found) 
	      waitpid(pid, &status, WUNTRACED); //calls wait
	  }
          continue;
	}
	
        if (!strcmp(args[0],"exit")) { 
          break; //break out of shell loop
        }

	else{
	  pid = getpid();   
          if((pid = fork ())== -1) { 
	    perror("FORK ERROR");
	    abort();
	  }
	  else if(pid == 0){
	    setenv("parent", getenv("shell"), 1); //setting parent
	    if(b == 1)
	      freopen(outFile, "w", stdout);
	    else if(c == 1)
	      freopen(outFile, "a+", stdout); 
	    
	    if(execvp (args[0], args) == -1){
	      perror("Execution child error");
	      abort();
	    } //exec in child
	  }else{                
	    if (!found) //wait
	      waitpid(pid, &status, WUNTRACED);
	  }	  
	}
	continue;
      }//if args[0]
    }//if fgets
  }//while feof
  return 0;
}//main

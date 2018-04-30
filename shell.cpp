#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/wait.h>
using namespace std;


string getexepath(){
  const char * path;
  char result[PATH_MAX];
  ssize_t c = readlink( "/proc/self/exe", result, PATH_MAX );
  if (c != -1) path = dirname(result);
    return path;
}



char** read(){
  char** argv = (char**)malloc(100);
  char*input; 
  char*input_tok;
  size_t argc = 0;//size of argv1
  size_t buf = 0;
  getline(&input, &buf, stdin);
  input_tok = strtok(input, " ");

  while(input_tok != NULL){
    argv[argc] = input_tok;
    argc++;
    input_tok = strtok(NULL, " ");
  }
  argv[argc] = NULL;
  return argv;  
}//read

bool execute(char**argv){
  pid_t pid;
  int stat;
  char const *path_str = "/bin/";
  int length = strlen(path_str) + strlen(argv[0]) + 1;
  char*path = (char*)malloc(length);
  strcpy(path, path_str);//copy the path string into path*/
  strcat(path, argv[0]);
  int i =0;
  cout << argv[0] <<", " << argv[1] << endl;
  if((pid = fork()) == -1) {
    perror("FORK ERROR");
  }
  else if(pid == 0){
    if(execvp("/bin/ls",argv) == -1){
      perror("EXEC CHILD ERROR"); 
    //in child
    }//if

  }else{
    while(!WIFEXITED(stat) && !WIFSIGNALED(stat)){
      waitpid(pid,&stat,WUNTRACED);
      wait(NULL);
      cout << "PARENT" << endl;
      
      //in parent
    }
  }  
  //  free(path);
  return true;
  
}

  


void loop(){
  char**argv;
  bool prompt = true;
  while(prompt){
    cout << "1730sh:" << getexepath() << "$ ";
    argv = read();    
    prompt = execute(argv);
    free(argv); 
  }
}

 

int main(){
  bool prompt = 0;
  loop();
  return 0;
}

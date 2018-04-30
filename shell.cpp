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

//bool prompt = true;
//string message;

string getexepath(){
  const char * path;
  char result[PATH_MAX];
  ssize_t c = readlink( "/proc/self/exe", result, PATH_MAX );
  if (c != -1) path = dirname(result);
    return path;
}

char** read(){

  char input_char[100];
  char** argv = (char**)malloc(100);
  char*input; 
  size_t argc = 0;//size of argv1
  size_t buf = 0;
  getline(&input, &buf, stdin);
  while(input != NULL){
    cout << input << endl;
    argv[argc] = input;
    argc++;
    
    input = strtok(NULL, " ");
    
  }
  cout << "line 43" << endl;
  
  argv[argc] = NULL;
  cout << "line 46" << endl;
  
  return argv;  
}//read

bool execute(char**argv){
  pid_t pid;
  int stat;
  char const *path_str = "/bin/";
  int length = strlen(path_str) + strlen(argv[0]) + 1;
  cout << "line 55aa" << endl;
  char*path = (char*)malloc(length);
  strcpy(path, path_str);//copy the path string into path*/
  strcat(path, argv[0]);
  cout << "line 55" << endl;
  int i =0;
  cout << "line 57"<< endl;

  cout << *argv << endl;
  cout << path << endl;
  if((pid = fork()) == -1) {
    perror("FORK ERROR");
  }
  else if(pid == 0){
    if(execvp(path,argv+1) == -1){
      perror("EXEC CHILD ERROR"); 
    }//if
    //in child
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
    cout << "line 86" << endl;
    prompt = execute(argv);
    free(argv); 
  }
}

 

int main(){
  bool prompt = 0;
  loop();
  return 0;
}

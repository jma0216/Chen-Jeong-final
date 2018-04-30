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

const int MAX_ARGS = 256;
char *argv[MAX_ARGS], *cmd1[MAX_ARGS], *cmd2[MAX_ARGS];
int argc;

//True for redirect, false for everything else.
bool parse_command(int, char**, char**, char**){
  int x = -1;
  bool result = false;
 
  for (int i = 0; i < argc; i++){
    if (strcmp(argv[i], ">>") == 0) { 
      result = true; //i/o redirect is true
      x = i;
    }
  }
  
  if (result == false) {
    // Go through the array of arguments up to the point where the
    // redirect was found and store each of those arguments in cmd1.
    for (int i = 0; i < x; i++)
      cmd1[i] = argv[i];
    
    int count = 0;
    for (int i = x+1; i < argc; i++) {
      cmd2[count] = argv[i];
      count++;
    }
 
    cmd1[x] = NULL;
    cmd2[count] = NULL;
  }//if redirect
  return result;
}

bool quit(string input) {
  // Lowercase the user input
  //  for (unsigned int i = 0; i < input.length(); i++)
  // input[i] = tolower(input[i]);
  return (input == "quit" || input == "exit" || input == "Exit" || input == "Quit" || input == "QUIT" ||input == "EXIT");
}

char** read_args(char **argv) {
  char *cstr;
  string arg;
  int argc = 0;

  // Read in arguments till the user hits enter
  while (cin >> arg) {
    
    if (quit(arg)) {
      cout << "Goodbye!\n";
      exit(0);
    }

    // Convert that std::string into a C string.
    cstr = new char[arg.size()+1];
    strcpy(cstr, arg.c_str());
    argv[argc] = cstr;

    // Increment our counter of where we're at in the array of arguments.
    argc++;

    // If the user hit enter, stop reading input.
    if (cin.get() == '\n')
      break;
  }

  // Have to have the last argument be NULL so that execvp works.
  argv[argc] = NULL;
  return argv;
}


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
    if(execvp(path,argv) == -1){
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


string getexepath(){
  const char * path;
  char result[PATH_MAX];
  ssize_t c = readlink( "/proc/self/exe", result, PATH_MAX );
  if (c != -1) path = dirname(result);
  return path;
}


int main(){
  bool run = true;
  char**command_args;
  while(run){
    cout << "1730sh:" << getexepath() << "$ ";
    command_args = read_args(argv);
    run = execute(command_args);

  }//while



}

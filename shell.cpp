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
using namespace std;

bool prompt = true;
string message;

string getexepath(){
  const char * path;
  char result[PATH_MAX];
  ssize_t c = readlink( "/proc/self/exe", result, PATH_MAX );
  if (c != -1) path = dirname(result);
    return path;
}

void read(){
  while(prompt){
    cout << "1730sh:" << getexepath() << "$ ";
    getline(cin, message);
    if(message == "exit"){
      cout << "Exiting shell..";
	prompt = false;
    }//if
  }//while
}//read

int main(){
  
    read();

  }


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "mytoc.h"
#include "myString.h"

int numWords;
char delim = ' ';
char** tokenizedString;

char* formatPotentialPath(char* path, char* executableName, char* tokenizedPath){
        path = copyString(path, tokenizedPath);
        path = mergeStrings(path, "/");
	path = mergeStrings(path, executableName);
	return path;
}

char* returnPATH(char** envp){

  char** tokens;
  unsigned int i;
  
  for (i=0; envp[i] != (char*)0; i++){
    tokens = myToc(envp[i], '=');
    if (stringCompare("PATH", tokens[0])){
      return tokens[1];
    }
    free(tokens);
  }
  
}

void printPathLook(char** envp){

  pid_t pid;
  int runP;

  if (!access(tokenizedString[0], F_OK)){
      pid = fork();
      if (pid == 0){
	fflush(NULL);
	runP = execve(tokenizedString[0], tokenizedString, (char* const*) envp);
	if (runP == -1){
	  printf("\tProgram Terminated With Exit Code: %d\n", runP);
	}
	exit(0);
      } else {
	wait(NULL);
	return;
      }
    }

    char* pathEnv = returnPATH(envp);
    char** tokenizedPath = myToc(pathEnv, ':');
    int i, success = 0, numPaths = numberOfWords(pathEnv, ':');

    for (i = 0; i < numPaths; i++) {
        char* potentialPath = formatPotentialPath(potentialPath, tokenizedString[0], tokenizedPath[i]);
        int found = access(potentialPath, F_OK);
        if (found == 0){
            copyString(tokenizedString[0], potentialPath);
            pid = fork();
            if (pid == 0){
              fflush(NULL);
              runP = execve(potentialPath, tokenizedString, envp);
	      if (runP == -1){
		printf("Program Terminated With Exit Code: %d\n", runP);
	      }
              exit(0);
            } else {
              wait(NULL);
            }
            success = 1;
	    free(potentialPath);
            break;
        }
	free(potentialPath);
    }

    if(!success){
        printf("\tCommand not found!\n");
    }

    for (i = 0; i < numPaths + 1; i++){
        free(tokenizedPath[i]);
    }
    
    free(tokenizedPath);
    free(pathEnv);
}

int main(int argc, char **argv, char**envp){

    int len = 1024, tocChoice = 1, i;
    char inputString[len];

    LOOP:

    write(1,"$ ", 2);
    fgets (inputString, len, stdin) ;

    if (stringCompare(inputString, "exit\n")){
        printf("\nEnd of Execution\n\n");
        return 0;
    }

    numWords = numberOfWords(inputString, delim);

    if (!numWords){
      goto LOOP;
    }

    tokenizedString = myToc(inputString, delim);
    printPathLook(envp);

    if (tocChoice){
        for (i = 0; i < numWords + 1; i++){
            free(tokenizedString[i]);
        }
    }
    
    free(tokenizedString);
    goto LOOP;

    return 0;
}

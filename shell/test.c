#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "myString.h"
#include "mytoc.h"

int numWords; // Stores number of words per input
char delim = ' '; // Delimiter for tokenizer

// Formats strings to create valid a file path and returns it
char* formatPotentialPath(char* path, char* executableName, char* tokenizedPath){
  path = copyString(path, tokenizedPath); // Copies path
  path = mergeStrings(path, "/"); // Appends the '/' character to the path
  path = mergeStrings(path, executableName); // Appends the executable name to the path
  return path; // Returns final path
}

// Traverses the environment variables, returning PATH 
char* returnPATH(char** envp){
  char** tokens; // To tokenize the enviroment variables
  unsigned int i;
  for (i=0; envp[i] != (char*)0; i++){
    tokens = myToc(envp[i], '='); // Separates enviroment variables by value and key
    if (stringCompare("PATH", tokens[0])){
      return tokens[1]; // Returns PATH when found
    }
    free(tokens);
  }
}

char** formatExecutableParameters(char** tokenizedString, char** envp){
  int runP; // For exit code
  char* potentialPath;
	
  if (!access(tokenizedString[0], F_OK)){
    return tokenizedString;
  }

  char* pathEnv = returnPATH(envp); 
  char** tokenizedPath = myToc(pathEnv, ':'); 
  unsigned int i, success = 0, numPaths = numberOfWords(pathEnv, ':');

  for (i = 0; i < numPaths; i++) {
    potentialPath = formatPotentialPath(potentialPath, tokenizedString[0], tokenizedPath[i]);
    int found = access(potentialPath, F_OK); // Determines if path and command exists
    // If path & command are valid, attempt to execute
    if (found == 0){
      goto FREE;
    }
    free(potentialPath); // Free the currently formatted path
  }
	
  potentialPath = malloc(sizeof(char) * 2);
  potentialPath[0] = '0';
  potentialPath[1] = '\0';
	
 FREE:
  tokenizedString[0] = copyString(tokenizedString[0], potentialPath);
  // Frees the tokenized path vector tokens
  for (i = 0; i < numPaths + 1; i++){
    free(tokenizedPath[i]);
  }
  free(tokenizedPath); 	// Frees the tokenized path vector 
  free(pathEnv); // Frees the PATH enviroment variable
  return tokenizedString;
}

int executePipedCommand(char** tokenizedString, char** envp, int in, int out){
  pid_t pid;
  if ((pid = fork()) == 0){
    if (in != 0){
      dup2(in, 0);
      close(in);
    }
    if (out != 1) {
      dup2(out, 1);
      close(out);
    }
    return execve(tokenizedString[0], tokenizedString, envp);
  }
  wait(NULL);
  return pid;
}


void pippedExecution(char** tokenizedCommands, char** envp, int numCommands){
  int pid, i, j, in = 0, stdinCopy = dup(0), stdoutCopy = dup(1);
  int fd[2];
  char** tokenizedString;
  
  for (i = 0; i < numCommands-1; i++){
    pipe(fd);

    numWords = numberOfWords(tokenizedCommands[i], ' ');
    tokenizedString = myToc(tokenizedCommands[i], ' ');

    if (stringCompare(tokenizedString[0], "cd")){
      if (numWords == 1)
	chdir("/root/");
      else 
	chdir(tokenizedString[1]);
      goto FREE;
    }
    
    tokenizedString = formatExecutableParameters(tokenizedString, envp);
    
    if (tokenizedString[0][0] != '0'){
      executePipedCommand(tokenizedString, envp, in, fd[1]);
      close(fd[1]);
      in = fd[0];
    } else {
      printf("\tCommand not found!\n");
      for (j = 0; j < numWords + 1; j++){
        free(tokenizedString[j]);
      }
      free(tokenizedString);
      goto RSTRFD;
    }
  FREE:
    for (j = 0; j < numWords + 1; j++){
      free(tokenizedString[j]);
    }
    free(tokenizedString);
  }

  if (in != 0){
    dup2(in, 0);
  }
    
  tokenizedString = myToc(tokenizedCommands[i], ' ');
  tokenizedString = formatExecutableParameters(tokenizedString, envp);
  pid = fork();
  
  if (pid == 0){
    fflush(NULL);
    execve(tokenizedString[0], tokenizedString, envp);
    exit(0);
  } else {
    RSTRFD: 
    dup2(stdinCopy, 0);
    dup2(stdoutCopy, 1);
    close(stdinCopy);
    close(stdoutCopy);
    fflush(NULL);
  }
}

void executeSingleCommand(char* inputString, char** envp){
  int i, numArg;
  char** tokenizedString;
  numArg = numberOfWords(inputString, ' ');
  tokenizedString = myToc(inputString, ' ');

  
  if (stringCompare(tokenizedString[0], "cd")){
    if (numArg == 1)
      chdir("/root/");
    else 
      chdir(tokenizedString[1]);
    goto FREE;
  }
  
  
  tokenizedString = formatExecutableParameters(tokenizedString, envp);
  if (tokenizedString[0][0] != '0'){
    int pid = fork();
    if (pid == 0){
      fflush(NULL);
      execve(tokenizedString[0], tokenizedString, envp);
      exit(0);
    } else {
      wait(NULL);
    }
  } else {
    printf("\tCommand not found!\n");
  }

 FREE:
  for (i = 0; i < numArg + 1; i++){
    free(tokenizedString[i]);
  }
  free(tokenizedString);
}

void checkForExecutables(char* inputString, char** envp){
  int numCommands = numberOfWords(inputString, '|'), i, j;
  if (numCommands == 1){
    executeSingleCommand(inputString, envp);
  } else {
    char** tokenizedCommands = myToc(inputString, '|');
    pippedExecution(tokenizedCommands, envp, numCommands);
    for (j = 0; j < numWords + 1; j++){
      free(tokenizedCommands[j]);
    }
    free(tokenizedCommands);
  }
}

int main(int argc, char **argv, char**envp){
  unsigned int len = 1024, i;
  char inputString[len];

  LOOP:
  write(1,"$ ", 2);
  fgets (inputString, len, stdin); // Read input from user

  // Built in exit funcion for the shell
  if (stringCompare(inputString, "exit\n")){
    printf("\nEnd of Execution\n\n");
    return 0;
  }

  // Determines the number of input words
  numWords = numberOfWords(inputString, delim);

  // If no input was provided, re-prompt
  if (!numWords){
    goto LOOP;
  }
  
  checkForExecutables(inputString, envp);
  wait(NULL);
  goto LOOP;

  return 0;
}

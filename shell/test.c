#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "myString.h"
#include "mytoc.h"
#include "errno.h"

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

// Receives argument vector, returns a properly formated argument vector if a valid 
// command was provided or "0" as the file path if the provided command was not found 
char** formatExecutableParameters(char** tokenizedString, char** envp){
  int runP; // For exit code
  char* potentialPath;
	
  // If the provided argument vector was already properly formatted, return it
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

// Execute commands, supporting standard piping
int executeCommand(char* inputString, char** envp, int pipe, int in, int out){
  int i, numArg, retVal = 0;
  char** tokenizedString;
  
  numArg = numberOfWords(inputString, ' '); // Calculate number of arguments 
  tokenizedString = myToc(inputString, ' '); // Tokenizes the input string into individual arguments 

  // If no arguments were provided, return
  if (!numArg) {
    return retVal;
  }
  
  // Checks for the special case of the cd command
  if (stringCompare(tokenizedString[0], "cd")){
    if (numArg == 1)
      chdir("/root/"); // If no path was specified, go to the root folder
    else 
      chdir(tokenizedString[1]); 
    retVal = 1;
    goto FREE; // Free malloc'd argument vector
  }
  
  // Finds absolute path for input command and returns a properly formatted argument vector
  tokenizedString = formatExecutableParameters(tokenizedString, envp);

  // Run only if provided command was found
  if (tokenizedString[0][0] != '0'){
    pid_t pid = fork();
    if (pid == 0){
      fflush(NULL);
      // If piping is taking place, redirect I/O
      if(pipe){
        // If pipe input isn't set to stdin
        if (in != 0){
          // Copy pipe input fd into stdin
          dup2(in, 0);
          // Close hanging file descriptor 
          close(in);
        }
        // If pipe output isn't set to stdin
        if (out != 1) {
          // Copy pipe output fd into stdout
          dup2(out, 1);
          // Close hanging file descriptor 
          close(out);
        }
      }
      // If in child, execute command
      if(execve(tokenizedString[0], tokenizedString, envp) == -1){
	printf("Program terminated with exit code: %d\n", errno);
      }
      exit(0);
    } else {
      wait(NULL); // Wait for child to be done before continuing execution
      retVal = 1;
    }
  } else {
    printf("\tCommand not found!\n");
  }

 FREE:
  // Free argument vector, to avoid memory leaks
  for (i = 0; i < numArg + 1; i++){
    free(tokenizedString[i]);
  }
  free(tokenizedString);
  return retVal;
}

// Provides I/O redirection for piped commands
void pipedExecution(char** tokenizedCommands, char** envp, int numCommands){
  int i, j, in = 0, out;
  int* fd = calloc(sizeof(int), 2);
  pid_t pid;
  // Stores the original file descriptors for stdin & stdout
  int stdinCopy = dup(0), stdoutCopy = dup(1); 
  for (i = 0; i < numCommands-1; i++){
    pipe(fd); // Sets pipe
    if(executeCommand(tokenizedCommands[i], envp, 1, in, fd[1])){
      close(fd[1]); // Close pipe output fd
      in = fd[0]; // Copy input fd from pipe
    } else {
      printf("\tCommand not found!\n");
      goto RSTRFD; // Restore default file descriptors
    }
  }
  
  // Copy pipe input fd into stdin
  if (in != 0){
    dup2(in, 0);
  }
  // Executes final command in the pipe chain
  executeCommand(tokenizedCommands[i], envp, 1, in, fd[1]);
  // Restores the default stdin & stdout fds
 RSTRFD: 
  dup2(stdinCopy, 0);
  dup2(stdoutCopy, 1);
}

// Determines if piping was instructed, handling it if it's the case
void checkForExecutables(char* inputString, char** envp){
  int numCommands = numberOfWords(inputString, '|'), i, j;
  if (countInstances(inputString, '|') && !numCommands){
    printf("\tUnexpected token |\n");
    return;
  }
  if (numCommands == 1){
    // If only a single command was provided, no piping will take place
    executeCommand(inputString, envp, 0, 0, 0); 
  } else {
    // Creates an executables vector
    char** tokenizedCommands = myToc(inputString, '|'); 
    pipedExecution(tokenizedCommands, envp, numCommands);
    // Frees executable vector after execution, to avoid memory loss
    free(tokenizedCommands);
  }
}

// Determines if background taks were instructed, handling if it's the case
void checkForBackground(char* inputString, char** envp){
  int numBackground = numberOfWords(inputString, '&'), i;

  //printf("numBackground: %d\n", numBackground);
  int inst = countInstances(inputString, '&');

  if(!numBackground && inst){
    return;
  }
  
  // If there will be no background execution
  if (numBackground == 1 && !inst){
    // Proceed to check for piping
    checkForExecutables(inputString, envp);
    return;
  }
  
  // If background tasks were specified, create a task vector
  char** backTask = myToc(inputString, '&');
  for (i = numBackground-1; i > 0; i--){
    int pid = fork();
    if (pid == 0){
      // Last task has highest priority, so it executes first
      checkForExecutables(backTask[i], envp);
      exit(0);
    } 
  }

   // Waits for other tasks to stop executing before running last command
  checkForExecutables(backTask[i], envp);
    
  // Free the task vector, to avoid memory leaks
  for (i = 0; i < numBackground; i++){
    free(backTask[i]);
  }
  free(backTask);
}

// Main thread of execution for Shell
int main(int argc, char **argv, char**envp){
  unsigned int len = 1024, i;
  char* inputString = (char*) calloc(sizeof(char), len);
  // printf("cwd: %s\n", getlogin_r(inputString, len));
 LOOP:;

  write(1,"$ ", 0); // Stopped it from outputing prompt to run testShell.sh
    
  //fgets(inputString, len, stdin); // Read input from user
  int bytesRead = read(0, inputString, len); // Read input from user

  // To end execution on end of file
  if (!bytesRead){
    return 0;
  }
    
  // Built in exit funcion for the shell
  if (stringCompare(inputString, "exit\n")){
    printf("End of Execution\n");
    return 0;
  }
   
  // Determines the number of input words
  numWords = numberOfWords(inputString, delim);
  // If no input was provided, re-prompt
  if (!numWords){
    goto LOOP;
  }
  // Attempt to execute provided commands
  checkForBackground(inputString, envp);
  wait(NULL);
  free(inputString);
  inputString = (char*) calloc(sizeof(char), len);
  goto LOOP;
  return 0;
  
}

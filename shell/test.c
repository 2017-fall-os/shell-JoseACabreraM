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

// Provides I/O redirection for piped commands
void pippedExecution(char** tokenizedCommands, char** envp, int numCommands){
  int pid, i, j, in = 0;
  int fd[2];
  char** tokenizedString;
  
  for (i = 0; i < numCommands-1; i++){
    pipe(fd); // Sets pipe 
    numWords = numberOfWords(tokenizedCommands[i], ' '); // Calculate the number of arguments 
    tokenizedString = myToc(tokenizedCommands[i], ' '); // Tokenizes the input string into individual arguments 
    // Checks for the special case of the cd command
    if (stringCompare(tokenizedString[0], "cd")){
      if (numWords == 1){
        chdir("/root/"); // If no path was specified, go to the root folder
      }
      else{
        chdir(tokenizedString[1]);
      }
      continue; 
    }
    // Finds absolute path for input command and returns a properly formatted argument vector
    tokenizedString = formatExecutableParameters(tokenizedString, envp); 
    // Stores the original file descriptors for stdin & stdout
    int stdinCopy = dup(0), stdoutCopy = dup(1); 
    // Run only if provided command was found
    if (tokenizedString[0][0] != '0'){
      executePipedCommand(tokenizedString, envp, in, fd[1]);
      close(fd[1]); // Close output 
      in = fd[0]; // Receive input
    } else {
      printf("\tCommand not found!\n");
      // Free argument vector, to avoid memory leaks
      for (j = 0; j < numWords + 1; j++){
        free(tokenizedString[j]);
      }
      free(tokenizedString);
      goto RSTRFD; // Restore default file descriptors
    }
    // Free argument vector for next command
    for (j = 0; j < numWords + 1; j++){
      free(tokenizedString[j]);
    }
    free(tokenizedString);
  }
  
  if (in != 0){
    dup2(in, 0);
  }
  // Free argument vector for final command
  for (j = 0; j < numWords + 1; j++){
    free(tokenizedString[j]);
  }
  free(tokenizedString);
  // Format the final argument vector
  tokenizedString = myToc(tokenizedCommands[i], ' ');
  tokenizedString = formatExecutableParameters(tokenizedString, envp);
  // Attempt to execute the final command
  pid = fork();
  if (pid == 0){
    fflush(NULL);
    execve(tokenizedString[0], tokenizedString, envp);
    exit(0);
  } else {
    // Restores the default file descriptors once final process executes
    RSTRFD: 
    dup2(stdinCopy, 0);
    dup2(stdoutCopy, 1);
    close(stdinCopy);
    close(stdoutCopy);
    fflush(NULL);
  }
}

// Execute a regular single command 
void executeSingleCommand(char* inputString, char** envp){
  int i, numArg;
  char** tokenizedString;
  
  numArg = numberOfWords(inputString, ' '); // Calculate number of arguments 
  tokenizedString = myToc(inputString, ' '); // Tokenizes the input string into individual arguments 

  // If no arguments were provided, return
  if (!numArg) {
    return;
  }
  
  // Checks for the special case of the cd command
  if (stringCompare(tokenizedString[0], "cd")){
    if (numArg == 1)
      chdir("/root/"); // If no path was specified, go to the root folder
    else 
      chdir(tokenizedString[1]); 
    goto FREE; // Free malloc'd argument vector
  }
  
  // Finds absolute path for input command and returns a properly formatted argument vector
  tokenizedString = formatExecutableParameters(tokenizedString, envp);

  // Run only if provided command was found
  if (tokenizedString[0][0] != '0'){
    int pid = fork();
    if (pid == 0){
      // If in child, execute command
      fflush(NULL);
      execve(tokenizedString[0], tokenizedString, envp);
      exit(0);
    } else {
      wait(NULL); // Wait for child to be done before continuing execution
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
}

// Determines if piping was instructed, handling it if it's the case
void checkForExecutables(char* inputString, char** envp){
  int numCommands = numberOfWords(inputString, '|'), i, j;
  if (numCommands == 1){
    // If only a single command was provided, no piping will take place
    executeSingleCommand(inputString, envp); 
  } else {
    // Creates an executables vector
    char** tokenizedCommands = myToc(inputString, '|'); 
    pippedExecution(tokenizedCommands, envp, numCommands);
    // Frees executable vector after execution, to avoid memory loss
    for (j = 0; j < numWords + 1; j++){
      free(tokenizedCommands[j]);
    }
    free(tokenizedCommands);
  }
}

void checkForBackground(char* inputString, char** envp){

  int numBackground = numberOfWords(inputString, '&'), i;

  // If there will be no background execution
  if (numBackground == 1){
    // Proceed to check for piping
    checkForExecutables(inputString, envp);
    return;
  }
  
  // If background tasks were specified, create a task vector
  char** backTask = myToc(inputString, '&');
  for (i = numBackground-1; i >= 1; i--){
    int pid = fork();
    if (pid == 0){
      // Last task has highest priority, so it executes first
      checkForExecutables(backTask[i], envp);
      exit(0);
    } else {
      wait(NULL);
      // Waits for task with higher priority to finish before executing
      checkForExecutables(backTask[i-1], envp);
    }
  }
  // Free the task vector, to avoid memory leaks
  for (i = 0; i < numBackground; i++){
    free(backTask[i]);
  }
  free(backTask);
}

// Main thread of execution for Shell
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
    // Attempt to execute provided commands
    checkForBackground(inputString, envp);
    goto LOOP;
  return 0;
}

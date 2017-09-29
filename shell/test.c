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

// Attempts to execute a command give the command name or 
// absolute path, as well as with any corresponding arguments. 
void executeCommand(char** envp){

	pid_t pid; // For forking 
	int runP; // For exit code

	// If absolute path was provided & exists, verified by access function, attempt to execute
	if (!access(tokenizedString[0], F_OK)){
		pid = fork(); // Fork & create child process
		// If in child, attemp to execute command
		if (pid == 0){
			fflush(NULL); // Clears the buffer 
			runP = execve(tokenizedString[0], tokenizedString, (char* const*) envp); // Attemps to execute command
			if (runP == -1){
				printf("\tProgram Terminated With Exit Code: %d\n", runP); // If command failed to execute, print exit code
			}
			exit(0);
		} else {
			wait(NULL); // If in parent wait for child process to terminate
			return;
		}
	}

    char* pathEnv = returnPATH(envp); // To store PATH enviroment variable
    char** tokenizedPath = myToc(pathEnv, ':'); // Tokenizes PATH
    unsigned int i, success = 0, numPaths = numberOfWords(pathEnv, ':');

	// Traverses available paths
    for (i = 0; i < numPaths; i++) {
		char* potentialPath = formatPotentialPath(potentialPath, tokenizedString[0], tokenizedPath[i]); // Provides proper path formatting
		int found = access(potentialPath, F_OK); // Determines if path and command exists
		// If path & command are valid, attempt to execute
		if (found == 0){
			copyString(tokenizedString[0], potentialPath); // Copies appropiate path into token vector, for the argv parameter
			pid = fork(); // Fork & create child process
			// If in child, attemp to execute command
			if (pid == 0){
				fflush(NULL); // Clears the buffer
				runP = execve(potentialPath, tokenizedString, envp); // Attemps to execute command
				if (runP == -1){
					printf("Program Terminated With Exit Code: %d\n", runP); // If command failed to execute, print exit code
				}
				exit(0);
			} else {
				wait(NULL);  // If in parent wait for child process to terminate
			}
			success = 1; // Command was found
			free(potentialPath); // Free the currently formatted path
			break; // Break out of loop, execution has happened
		}
		free(potentialPath); // Free the currently formatted path
    }

	// Print message if command was not found 
    if(!success){
        printf("\tCommand not found!\n");
    }

	// Frees the tokenized path vector tokens
    for (i = 0; i < numPaths + 1; i++){
        free(tokenizedPath[i]);
    }
	
    free(tokenizedPath); 	// Frees the tokenized path vector 
    free(pathEnv); // Frees the PATH enviroment variable
}

/*
int countPipingOperators(char* inputString){
  unsigned int opCount = 0;
  char* temp;
  for (temp = inputString; *temp != '\0'; temp++){
    if (*temp == '>' || *temp == '<' || *temp == '|'){
      opCount++;
    }
  }
  return opCount;
}
*/

char** formatExecutableParameters(char** tokenizedString, char** envp){
	int runP; // For exit code
	char* potentialPath;
	
	if (!access(tokenizedString[0], F_OK)){
		return tokenizedString;
	}

    char* pathEnv = returnPATH(envp); 
    char** tokenizedPath = myToc(pathEnv, ':'); 
    unsigned int i, success = 0, numPaths = numberOfWords(pathEnv, ':');

	// Traverses available paths
    for (i = 0; i < numPaths; i++) {
		potentialPath = formatPotentialPath(potentialPath, tokenizedString[0], tokenizedPath[i]); // Provides proper path formatting
		int found = access(potentialPath, F_OK); // Determines if path and command exists
		// If path & command are valid, attempt to execute
		if (found == 0){
			goto FREE;
		}
		free(potentialPath); // Free the currently formatted path
    }
	
	potentialPath = malloc(sizeof(char) * 2);
	potentialPath[0] = '0';
	potentialPath[1] = '\0'
	
	FREE: 
	copyString(tokenizedString[0], potentialPath);
	// Frees the tokenized path vector tokens
	for (i = 0; i < numPaths + 1; i++){
        free(tokenizedPath[i]);
    }
    free(tokenizedPath); 	// Frees the tokenized path vector 
	free(pathEnv); // Frees the PATH enviroment variable
	return tokenizedString;
}

void checkForExecutables(char* inputString, char** envp){

  //executeCommand(envp);
  int numCommands = numberOfWords(inputString, '|'), i, j;
  char** tokenizedString;
  
  printf("\nnumCommands:%d\n", numCommands);
  
  if (numCommands == 1){
    tokenizedString = formatExecutableParameters(tokenizedString, envp);
	if (tokenizedString[0] != '0'){
		execve(tokenizedString[0], tokenizedString, envp); // Attemps to execute command
	} else {
        printf("\tCommand not found!\n");
	}
    // Free token vector once execution has finished
    for (i = 0; i < numWords + 1; i++){
        free(tokenizedString[i]);
    }
    free(tokenizedString);// Frees the tokenized path vector tokens
  } else {
	char** tokenizedCommands = myToc(inputString, '|');
	pippedExecution(tokenizedCommands, envp, numCommands);
	for (j = 0; j < numWords + 1; j++){
		free(tokenizedCommands[j]);
	}
	free(tokenizedCommands);
  }

}

void pippedExecution(char** tokenizedCommands, char** envp, int numCommands){

	int pid, status;
	int fd[2];
	pipe(fd);

	LOOP:
		tokenizedString = formatExecutableParameters(tokenizedCommands[numCommands-1], envp);
		pid = fork();
		if (pid == 0){
			close(fd[0]);
			executeCommand(envp);
			return;
		} else {
			close(fd[1])
			numCommand--;
			// Free token vector once execution has finished
			for (j = 0; j < numWords + 1; j++){
				free(tokenizedString[j]);
			}
			free(tokenizedString);// Frees the tokenized path vector tokens 
			if (numCommands > 0){
				GOTO LOOP;
			} else {
				return;
			}
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

    // Tokenize input, separate by spaces
    //tokenizedString = myToc(inputString, delim);
    // Attempt to execute provided input
    checkForExecutables(inputString, envp);


    goto LOOP;

    return 0;
}

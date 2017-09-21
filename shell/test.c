#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mytoc.h"

char delim = ' ';
char** tokenizedString;

void printPathLook(char* executableName, char** envp, int pLength){

    if (!access(executableName, F_OK)){
        printf("Absolute Path \n");
        //char* const * args = (char* const*) myToc(executableName, '\0');
        //char* const argv[] = {executableName, 0};
        //printf("%s\n", args[0]);
	
        execve(executableName, tokenizedString, (char* const*) envp);
        return;
    }

    char *dup = strdup(getenv("PATH"));
    int numWords = numberOfWords(dup, ':');
    char** tokenizedPath = myToc(dup, ':');

    int i, success = 0;

    for (i = 0; i < numWords; i++) {

        char potentialPath[1024];

        strcpy(potentialPath, tokenizedPath[i]);
        strcat(potentialPath, "/");
        strcat(potentialPath, executableName);

	//strcat(potentialPath, ".bin");
        //printf("i: %d ", i);

	/*
        char* temp = potentialPath;
        while(*temp != '\0'){
            write(1, temp, 1);
            temp++;
        }
        write(1,"\n", 1);
	*/

        int found = access(potentialPath, F_OK);

        if (found == 0){
	  
	    char* const argv[] = {potentialPath, 0};

	    free(tokenizedString[0]);

	    int pPathLength = 0;
	    char* temp = potentialPath;
	    while(*temp != '\0'){
	      pPathLength++;
	      temp++;
	    }
	    
	    tokenizedString[0] = malloc(sizeof(char) * (pPathLength + 1));
	    strcpy(tokenizedString[0], potentialPath);
	    //printf("\n%s\n", tokenizedString[0]);
	    /*
	    int k;
	    for (k = 0; k < pLength; k++){
	      printf("%s\n", tokenizedString[k]);
	    }
	    */
	    
	    pid_t pid = fork();
	    
	    if (pid == 0){
	      fflush(NULL);
	      execve(potentialPath, tokenizedString, envp);
	      exit(0);
	    } else {
	      wait(NULL);
	    }
     
            success = 1;
            break;
        }

        //write(1,"\n", 1);
    }

    if(!success){
        printf("\n\tExecutable not found!\n\n");
    }
    
    for (i = 0; i < numWords + 1; i++){
        free(tokenizedPath[i]);
    }

    free(tokenizedPath);
    free(dup);
}

int main(int argc, char **argv, char**envp){

    int len = 1024, tocChoice = 1;
    char inputString[len];

    LOOP:

    write(1,"$ ", 2);
    fgets (inputString, len, stdin) ;

    // To end execution
    if (stringCompare(inputString, "exit\n")){
        printf("\nEnd of Execution\n\n");
        return 0;
    }

    int numWords = numberOfWords(inputString, delim);

    if (!numWords){
      goto LOOP;
    }
    
    tokenizedString = myToc(inputString, delim);
    int i;

    /*
    // Prints the token vector, token by toke, character by character
    write(1,"\n", 1);
    printf("------------------------------------------\n");
    for (i = 0; i < numWords; i++){
        char* temp = tokenizedString[i];
        while(*temp != '\0'){
            write(1, temp, 1);
            temp++;
        }
        write(1,"\n", 1);
    }
    printf("------------------------------------------\n");
    write(1,"\n", 1);
    */

    printPathLook(tokenizedString[0], envp, numWords);

    // Only free each token memory if they were allocated using the regular tokenizer
    if (tocChoice){
        for (i = 0; i < numWords + 1; i++){
            free(tokenizedString[i]);
        }
    }

    // Free the pointer array from memory
    free(tokenizedString);
    goto LOOP;

    return 0;
}

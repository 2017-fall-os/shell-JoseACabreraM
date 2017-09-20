#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mytoc.h"

char delim = ' ';
char** tokenizedString;

void printPathLook(char* executableName, char** envp){

    if (!access(executableName, F_OK)){
        printf("Absolute Path \n");
        const char** args = (const char**) myToc(executableName, '\0');
        const char *argv[] = {executableName, 0};
        printf("%s\n", args[0]);
        execve(executableName, &argv[0], (const char**) envp);
        return;
    }

    char *dup = strdup(getenv("PATH"));

    int numWords = numberOfWords(dup, ';');
    //printf("numWords: %d\n", numWords);
    char** tokenizedPath = myToc(dup, ';');

    int i, success = 0;

    for (i = 0; i < numWords; i++) {

        char potentialPath[1024];

        strcpy(potentialPath, tokenizedPath[i]);
        strcat(potentialPath, "\\");
        strcat(potentialPath, executableName);
        strcat(potentialPath, ".exe");
        //printf("i: %d ", i);


        char* temp = potentialPath;
        while(*temp != '\0'){
            write(1, temp, 1);
            temp++;
        }
        write(1,"\n", 1);


        int found = access(potentialPath, F_OK);

        if (found == 0){
            //const char** args = (const char**) myToc(dup, '\0');
            const char *argv[] = {potentialPath, 0};
            //printf("\n%d", found);
            execve(potentialPath, &argv[0], 0);
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

    printPathLook(tokenizedString[0], envp);

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

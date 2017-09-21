#include <unistd.h>
//#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mytoc.h"

char delim = ' ';
char** tokenizedString;

int stringLength(char* source){
    char* temp = source;
    int length = 0;
    while(*temp != '\0'){
        length++;
        temp++;
    }
    return length;
}

char* copyString(char* dest, char* source){
    unsigned int i, sourceLength = stringLength(source);
    dest = malloc(sizeof(char) * (sourceLength+1));
    for (i = 0; i < sourceLength; i++){
        dest[i] = source[i];
    }
    dest[sourceLength] = '\0';
    return dest;
}

char* mergeStrings(char* base, char* extra){
    unsigned int msLength = stringLength(base) + stringLength(extra) + 1, baseLength = stringLength(base), i;
    char* mergedString = malloc(sizeof(char) * msLength);

    for (i = 0; i < msLength; i++){
        if (i < baseLength){
            mergedString[i] = base[i];
        } else {
            mergedString[i] = extra[i-baseLength];
        }
    }

    return mergedString;
}

void printPathLook(char* executableName, char** envp, int pLength){

    if (!access(executableName, F_OK)){
        printf("Absolute Path \n");
        execve(executableName, tokenizedString, (char* const*) envp);
        return;
    }

    char *dup = strdup(getenv("PATH"));
    int numPaths = numberOfWords(dup, ':');
    char** tokenizedPath = myToc(dup, ':');

    int i, success = 0;

    for (i = 0; i < numPaths; i++) {

        char potentialPath[1024];

        strcpy(potentialPath, tokenizedPath[i]);
        strcat(potentialPath, "/");
        strcat(potentialPath, executableName);

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

    int len = 1024, tocChoice = 1, i;
    char inputString[len];

    for (i=0; envp[i] != (char*)0; i++)
        printf("envp[%d] = \"%s\"\n", i, envp[i]);

    LOOP:

    write(1,"$ ", 2);
    fgets (inputString, len, stdin) ;

    if (stringCompare(inputString, "exit\n")){
        printf("\nEnd of Execution\n\n");
        return 0;
    }

    int numWords = numberOfWords(inputString, delim);

    if (!numWords){
      goto LOOP;
    }

    tokenizedString = myToc(inputString, delim);
    //printPathLook(tokenizedString[0], envp, numWords);

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

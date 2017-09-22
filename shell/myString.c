#include <stdlib.h>

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
    char* baseCopy = copyString(baseCopy, base);
    base = realloc(base, sizeof(char) * msLength);
    for (i = 0; i < msLength; i++){
        if (i < baseLength){
            base[i] = baseCopy[i];
        } else {
            base[i] = extra[i-baseLength];
        }
    }
    free(baseCopy);
    return base;
}

// Compares two strings, returns 1 if they're equal, 0 otherwise
int stringCompare(char* fString, char* sString){
    int i = 0;
    while (fString[i] == sString[i]){
        if (fString[i] == '\0' || sString[i] == '\0'){
            break;
        }
        i++;
    }
    return(fString[i] == '\0' && sString[i] == '\0');
}

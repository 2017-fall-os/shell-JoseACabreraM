#include <stdlib.h>

// Calculates the length of string, not accounting for '\0'
int stringLength(char* source){
    char* temp = source;
    int length = 0;
	// Traverses the string's characters
    while(*temp != '\0'){
        length++;
        temp++;
    }
	// Returns character count
    return length;
}

// Copies the value of one string into another one
char* copyString(char* dest, char* source){
    unsigned int i, sourceLength = stringLength(source); // Calculate size of the string to copy
    dest = malloc(sizeof(char) * (sourceLength+1)); // Allocate memory for destination string
    // Copy the strings
	for (i = 0; i < sourceLength + 1; i++){
        dest[i] = source[i];
    }
	// Return the copied string
    return dest;
}

// Given two strings, appends the second string to the first one
char* mergeStrings(char* base, char* extra){
    unsigned int msLength = stringLength(base) + stringLength(extra) + 1, baseLength = stringLength(base), i; // Calculate length of merged string
    char* baseCopy = copyString(baseCopy, base); // Create a copy of the base
    base = realloc(base, sizeof(char) * msLength); // Reallocate memory for the base string to account for the second one's characters
    // Merge strings together
	for (i = 0; i < msLength; i++){
        if (i < baseLength){
            base[i] = baseCopy[i];
        } else {
            base[i] = extra[i-baseLength];
        }
    }
    free(baseCopy); // Free the copied based string
    return base; // Return merged string
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

// Counts the number of instances of c within iString
int countInstances(char* iString, char c){
  int count = 0;
  char* temp;
  for(temp = iString; *temp != '\0'; temp++){
    if(*temp == c){
      count++;
    }
  }
  return count;
}

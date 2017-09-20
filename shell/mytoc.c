#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// Takes a char pointer and changes it based on user input
// Used to change the tokenizer delimiter character
void changeDelimiter(char* delim){
    printf("\nInput a new delimiter character: ");
    scanf("%[^\n]%*c", delim);
    write(1,"\n", 1);
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

// Given a string, counts the number of words
// Words are considered as sequence of characters, separated by the input delimiter
int numberOfWords(char* inputString, char delim){

    // Directly return zero if the first character is a newspace
    // To avoid doing unnecessary work if user just pressed enter
    if (*inputString == '\n'){
        return 0;
    }

    char* temp = inputString; // To traverse the input string
    int i = 0; // Keep track of number of words
    int wordFound = 0;

    while(1){
        // Considers a word has been found if the current character is not currenlty a delimiter or newspace
        if (*temp != delim && *temp != '\n' && *temp != '\0'){
            wordFound = 1;
        }
        // If a word has been found and a delimiter or newspace is reached, consider that word as accounted for
        if ((wordFound && *temp == delim) || (wordFound && *temp == '\n')){
            i++;
            wordFound = 0; // Not considering word until a non delimiter char is found
        }
        // Reached end of input string, return the word counter
        if (*temp == '\0'){
            return i + wordFound;
        }
        temp++;
    }

    return i;
}

// Receives a string and a delimiter character, returns an pointer vector
// which contains pointers to freshly allocated copies of the individual words
// within the input string. Words are considered as sequence of characters, separated by the input delimiter
char** myToc(char* inputString, char delim){

    int numWords = numberOfWords(inputString, delim);
    char** tokenizedStrings = (char**) malloc(sizeof(char*) * (numWords + 1));

    int currentStringLength = 0, i = 0, readingWord = 1, j;
    char* temp = inputString;
    char* wordStart = inputString;

    while(1){

        // If end of input string was reached without currently reading a word
        if(*temp == '\0' && readingWord){
            break;
        }

        // If currently not in a delimiter, newspace, or end of string, increment current word length counter
        if(*temp != delim && *temp != '\n' && *temp != '\0'){
            currentStringLength++;
            // If at the start of a word, save the pointer for copying when lenght is calculated
            if (readingWord){
                readingWord = 0;
                wordStart = temp;
            }
        }

        // If a delimiter or end of string has been reached and it's not the start of a new word
        if((*temp == delim || *temp == '\0' ) && !readingWord){
            tokenizedStrings[i] = (char*) malloc(sizeof(char) * (currentStringLength + 1)); // Allocate the necessary space for the new word
            // Using the pointer at the start of the word, copy its contents into the vector
            for (j = 0; j < currentStringLength; j++){
                tokenizedStrings[i][j] = *wordStart;
                wordStart++;
            }
            tokenizedStrings[i][currentStringLength] = '\0'; // Append the end of string char
            currentStringLength = 0; // Reset word length counter for next word
            i++;
            readingWord = 1; // Assume a new word is incoming

            // When the input string has been exhausted, break out of the loop
            if(*temp == '\0'){
                break;
            }

        }
        temp++;
    }

    tokenizedStrings[numWords] = '\0'; // Append end of string character to the token vector

    return tokenizedStrings; // Return token vector
}

// Alternative tokenizer implementation that doesn't copy words, it modifies the input string
char** myToc2(char* inputString, char delim){

    int numWords = numberOfWords(inputString, delim);
    char** tokenizedStrings = (char**) malloc(sizeof(char*) * (numWords + 1));
    int insertWord = 1;
    int i = 0;
    char* temp = inputString;

    while(1){
        // When original end of string is reached, copy last word address and return token vector
        if (*temp == '\0'){
            tokenizedStrings[i] = temp;
            return tokenizedStrings;
        }
        // If at the start of a new word, not counting the delimiter, copy the current address into the token vecotr
        if (insertWord && *temp != delim){
            tokenizedStrings[i] = temp;
            i++;
            insertWord = 0; // No longer at the start of a word
        }
        // If a delimiter or newspace character is reached, replace it by end of string character
        if (*temp == delim || *temp == '\n'){
            insertWord = 1;
            *temp = '\0';
        }
        temp++;
    }

    return tokenizedStrings;
}

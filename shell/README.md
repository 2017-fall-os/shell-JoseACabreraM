This directory contains:

A program that implements a basic Unix shell. The user should be able to run most of the default Linux commands just by typing the command's name, as well as 
by providing the command's absolute path. Basic command argument functionality is also included, except in cases where String are passed as arguments with spaces 
inside the String. The shell includes the following built-in function: 

  exit - End execution of the program.

The shell now supports piping by providing it commands that are in the form of: 

  $ command1 | command2
  
The shell now supports background tasking by providing it commands that are in the form of: 

  $ command1 & command2


From meeting with Adrian on 10/04/2017

     * Removed the "$ " prompt to make the shell work with testShell.sh
     * Added the check for end of file by counting the number of bytes read with the read() function
     * We determined that although the "two commands, full pathname"
       & "two commands, blank line in-between, full pathname" test cases fail the shell is able to handle them correctly
     * Same case with the cd command, which the shell handles properly but the script marks as failed
     * Background tasks was a similar story, fixed the bug we discussed during out meeting where it failed
       to recognize 'command &' but would recognize 'command1 & command2'
  
This program contains the following files:

mytoc.c - Contains the tokenizer and its helper functions. 
myString.c - Contains personal String helper functions for use with the shell. 
test.c - Executes the shell and it's helper functions. 
myToc.h - myToc.c.c Header file.
myString.h - myString.c Header file.

To test it, try:

$ make load

To delete binaries:

$ make clean
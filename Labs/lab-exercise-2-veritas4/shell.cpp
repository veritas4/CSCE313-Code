/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <iostream>
using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};
    
    
    // TODO: add functionality
    // Create pipe
    int fd[2];
    if (pipe(fd) < 0) {
        cout << "pipe creation error" << endl;
        exit(-1);
    }

    // Create child to run first command
    // In child, redirect output to write end of pipe
    // Close the read end of the pipe on the child side.
    // In child, execute the command
    pid_t pid = fork();
    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], 1);   
        execvp(cmd1[0], cmd1);
    }
    
    // Create another child to run second command
    // In child, redirect input to the read end of the pipe
    // Close the write end of the pipe on the child side.
    // Execute the second command.
    pid = fork();
    if (pid == 0) {
        close(fd[1]);
        dup2(fd[0],0);
        execvp(cmd2[0],cmd2);
    }

    // Reset the input and output file descriptors of the parent.
    // dup2(0,3); // (target,src)
    // dup2(1,4); // (target,src)
}

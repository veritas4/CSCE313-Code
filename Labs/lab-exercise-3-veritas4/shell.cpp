/****************
LE2: Basic Shell
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <sys/wait.h> // wait
#include "Tokenizer.h"
#include <iostream>
using namespace std;

int main () {
    // TODO: insert lab exercise 2 main code here
    
    // Save the default file descriptors of the parent
    dup(0);         
    dup(1);
    string input;

    while(true) {

        cout << "Enter the command(s): ";
        getline(cin, input);

        if (input.compare("exit") == 0 || input.compare("Exit") == 0) {
            break;
        }

        Tokenizer tokenize(input);          // Tokenize the input into commands
        // for (auto &j : tokenize.commands) {
        //      for (auto &i : j->args) {
        //          cout << i << endl;
        //      }
        // }
        for (auto &i : tokenize.commands) { // for each command, create a fork and pipe

            int fd[2];
            if (pipe(fd) < 0) {
                cout << "pipe creation error" << endl;
                exit(-1);
            }

            pid_t pid = fork();
            if (pid == 0) {
                if (i != tokenize.commands.back()) {   // When executing the last command, we don't want to redirect output
                    dup2(fd[1],1);                     // In child, redirect output to the write end of the pipe.
                }
                close(fd[0]);

                char** args = new char*[i->args.size() + 1];    // Make an array char * to store args
                for (size_t j = 0; j < i->args.size(); j++) {   // Loop through args<std::string>
                    args[j] = (char *)i->args[j].c_str();       // Convert string to (char *)
                }
                args[i->args.size()] = nullptr;                 // add nullptr to last arg for execvp()

                execvp(args[0],args);

                delete[] args;
            }
            else {
                dup2(fd[0],0);      // Redirect the SHELL(parent) input to the read end of the pipe
                close(fd[1]);       // Close the write end of the pipe
                
                int status;
                if (i == tokenize.commands.back()) {  // If executing last command, wait for child process
                    //cout << "waiting" << endl;
                    waitpid(pid,&status,0);
                }
            }
        }
        // Reset the input and output file descriptors of the parent.
        dup2(3,0); // (oldfd,newfd)
        dup2(4,1); // (oldfd,newfd)
    }
    
}
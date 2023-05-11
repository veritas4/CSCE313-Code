#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include <vector>
#include <string>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"
#define CYAN    "\33[0;36m"
#define PURPLE  "\33[0;35m"

using namespace std;

int main () {

    dup(0);
    dup(1);
    char newpath[250];

    for (;;) {

        time_t t = time(0);
        string time_now = ctime(&t);
        char path[250];
        getcwd(path,sizeof(path));

        cout << YELLOW << time_now.substr(4,15) << " " << PURPLE << getenv("USER") << ":" << CYAN <<path << "$" << NC << " ";
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        size_t dollar_sign = input.find("$");
        if (dollar_sign != string::npos) {
            size_t open_index = dollar_sign + 1;
            size_t close_index = input.find_first_of(")");
            string left_expr = input.substr(0,dollar_sign);
            string expression = input.substr(open_index + 1, close_index - open_index - 1);
            string right_expr = input.substr(close_index + 1, input.length() - close_index - 1);

            cout << left_expr << endl;
            cout << expression << endl;
            cout << right_expr << endl;

            return 0;
        }
        

        // get tokenized commands from user input
        // char newpath[250];
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }   
        if (tknr.commands.at(0)->args[0] == "cd") {
            if (tknr.commands.at(0)->args[1] != "-") {
                getcwd(newpath,sizeof(newpath));
                chdir(tknr.commands.at(0)->args[1].c_str());
            }
            else {
                if (newpath[0] != '\0') {
                    chdir(newpath);
                }
                else {
                    chdir(path);
                } 
            }
            continue;
        }

        for (auto command : tknr.commands) {

            int fd[2];

            if (pipe(fd) < 0) {
                cout << "pipe creation error" << endl;
                exit(-1);
            }

            // fork to create child
            pid_t pid = fork();
            //pids.push_back(pid);
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }
            int input_fd = 0;
            int output_fd = 1;

            
            if (pid == 0) {  // if child, exec to run command
                if (command->hasInput() && command->hasOutput()) {
                    input_fd = open(command->in_file.c_str(), O_RDONLY, 0700);
                    output_fd = open(command->out_file.c_str(), O_WRONLY|O_CREAT, 0700);
                    dup2(input_fd,0);
                    dup2(output_fd,1);
                }
                else if (command->hasInput()) {
                    input_fd = open(command->in_file.c_str(), O_RDONLY, 0700);  
                    dup2(input_fd,0); 
                }
                else if (command->hasOutput()) {
                    output_fd = open(command->out_file.c_str(), O_WRONLY|O_CREAT,0700);
                    dup2(output_fd,1);
                }
                if (command != tknr.commands.back()) {
                    dup2(fd[1],output_fd);
                }
                close(fd[0]);

                char** args = new char*[command->args.size() + 1];
                for (size_t i = 0; i < command->args.size(); i++) {
                    args[i] = (char *) command->args[i].c_str();
                }
                args[command->args.size()] = nullptr;


                if (execvp(args[0], args) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }
                delete[] args;
            }
            else {  // if parent, wait for child to finish
                dup2(fd[0],input_fd);
                close(fd[1]);

                int status = 0;
                if (!command->isBackground()) {
                    waitpid(pid, &status, 0);          
                }
                else {
                    signal(SIGCHLD,SIG_IGN);
                    //waitpid(pid,&stat,WNOHANG);
                }
                
                if (status > 1) {  // exit if child didn't exec properly
                    exit(status);
                }
            }
        }
        dup2(3,0);
        dup2(4,1);
    }
}

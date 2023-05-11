#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>


int main(int argc, char** argv) {
    int n = 1, opt;
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
        case 'n':
            n = atoi(optarg);
            break;
        }
    }

    /*
    1. TODO: fork n child processes and run p1-helper on each using execvp
        > note: we need to synchronize the child processes to print in the desired order
        > note: sleep(..) does not guarantee desired synchronization
        > note: check "raise" system call
        > note: check "WUNTRACED" flag for "waitpid"
    */
   std::vector<pid_t> child_pids;
   for (int i = 0; i < n; i++) {
        
        pid_t pid = fork();
        child_pids.push_back(pid);

        if (pid < 0) {
            std::cout << "Fork failed, child process could not be created" << std::endl;
            exit(0);
        }
        else if (pid == 0) {
            raise(SIGSTOP);     // Send signal to yourself to stop running the process(stop child processes)
            char *args[] = {(char *) "./p1-helper", NULL};
            execvp(args[0],args);
        }
   }
    
    /* 
    2. TODO: print children pids 
    */
    for (auto i : child_pids) {
        if (i == child_pids.back()) {
            std::cout << i << std::endl;
        }
        else {
            std::cout << i << " ";
        }
    }

    fflush(stdout);             // DO NOT REMOVE: ensures the first line prints all pids


    /* 
    3. TODO: signal children with the reverse order of pids 
        > note: take a look at "kill" system call 
    */
    int status;
    std::sort(child_pids.begin(),child_pids.end(), std::greater<__pid_t>());
    for (auto i : child_pids) {
        kill(i,SIGCONT);     // Send a signal to the children to continue running its process
        waitpid(i,&status,0);   
    }
    
    printf("Parent: exiting\n");

    return 0;
}
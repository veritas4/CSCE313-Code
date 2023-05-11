#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
char *custom_env[] = {"USER=Pikachu", "PATH=/tmp", NULL};

int main(void)
{
    // TODO
    printf("Running parent process PID: %d\n\n", getpid());

    pid_t first_child = fork();
    if (first_child < 0) {
        printf("Error creating child process\n");
        exit(-1);
    }
    else if (first_child == 0) {
        printf("Running child process PID: %d\n", getpid());
        const char *filepath = "echoall";
        const char *arg1 = "Bandicoot";
        const char *arg2 = "Pacman";
        execle(filepath, filepath, arg1, arg2, NULL, custom_env);
    }
    waitpid(first_child,NULL,0);

    pid_t second_child = fork();
    if (second_child < 0) {
        printf("Error creating child process\n");
        exit(-1);
    }
    else if (second_child == 0) {
        printf("Running child process PID: %d\n", getpid());
        const char *filepath = "echoall";
        const char *arg1 = "Spyro"; 
        execlp(filepath,filepath, arg1, NULL);
    }
    waitpid(second_child,NULL,0);

    exit(0);
}
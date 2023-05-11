#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#define MAX_MESSAGE 256

long long unsigned int hash(int seed, char* buf, int nbytes) {
    long long unsigned int H = seed; 
    for (int i = 0; i < nbytes; ++i) 
        H = H * 2 * buf[i] + 1;
    return H;
}

int main(int argc, char** argv) {
	
	// TODO: create pipe 
    int P1[2];
    int P2[2];
    for (int i = 0; i < argc; i++) {
        //std::cout << argv[i] << std::endl;
    }

    if (pipe(P1) < 0 || pipe(P2) < 0) {
        std::cout << "pipe creation error" << std::endl;
        exit(-1);
    }

    char* message = argv[1];
    char buf[MAX_MESSAGE];
    int pid = fork();
    if (pid == 0) {
        // TODO: read from parent
        close(P1[1]);
        close(P2[0]);

        int bytes  = read(P1[0],buf,sizeof(buf));
        // TODO: compute hash 
        long long unsigned int h = hash(getpid(), buf, bytes);

        // TODO: send hash to parent 
        write(P2[1],&h,sizeof(h));
    }
    else {
        // TODO: write to child 
        close(P1[0]);
        close(P2[1]);
        
        write(P1[1],message,sizeof(message));
        // TODO: get hash from child 
        long long unsigned int hrecv;

        long long unsigned int h;
        read(P2[0],&hrecv,sizeof(h));
        // TODO: calculate hash on parent side
        h = hash(pid,message,sizeof(message));
		
		// print hashes; DO NOT change
        printf("%llX\n", h);
        printf("%llX\n", hrecv);
    }

    return 0;
}
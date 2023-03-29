#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
// make all test
int main(int argc, char * const argv[]) {
//    if (argc != -1){
//        return;
//    }
    /*int err = errno;
    perror(message);
    exit(err);*/
    /* If there are no additional command line arguments, 
    your program should exit with an exit status equal to EINVAL. 
            Otherwise, you should execute the programs in argv[1], ..., argv[argc - 1] as new processes. */
    if (argc == 1 ){
        return EINVAL;
    }
    int i,rv = EXIT_SUCCESS;
//    int rv =  EXIT_SUCCESS;
    int fd[2] = {-1,-1}; /// file descriptor is array of two ints containing fd status
    
    /* a pipe should connect argv[1]'s standard output to argv[2]'s standard input (if there are at least two processes). The standard input of 
     * first new process must be the same as the standard input of the parent process (pipe).*/
    // gen pointer to a processid
    pid_t *pids = (pid_t*) malloc(sizeof(pid_t));
    // place the progs from argv  into pipeline
    for(i = 1; i < argc; i++)
    {
        int programFd[2] = {-1,-1};
        const char* program = argv[i]; // program name run str
//        printf("%c", argv[i]);
        
        /* if there is a pipe existing  we set the first pipe to fd[0]*/
        if (fd[1] != -1){
            
            programFd[0] = fd[0];
        }
        // printf("its quit: %s", argv[1]);
        
    
        // if i --> not exited i.e we have other commands in the commandline and havent reached the last arg
        if (i < (argc-1))
        {
        // check if err
            if(pipe(fd) == -1)
            {
                perror("pipe");
                exit(errno);
            }
            // push command out of pipe
            programFd[1] = fd[1];
        } 
        else
        { // if the program has reaced end of testprogs
            // set programFd to the og val
            programFd[1] = -1;
            
            
        }
        // call fork at this point: 
        const pid_t pid = fork();
        // two process created for chile and parent. 
        if (pid > 0)
        {
            // parentID, if its not exiting then close the pipe
            if(programFd[0] != -1){
                close(programFd[0]);
            }
            // if the next child not texited either then close as well
            if(programFd[1] != -1){
                close(programFd[1]);
            }
            // locate prev pid and set it to parent
            pids[i-1] = pid;
            
        }else
        {
            
            // we are now going to copy the input of one pipre to outp of other
            // duplicate file descriptor if open to the line, check if exists or not
            if (  ((programFd[0] != -1 ) && (dup2(programFd[0],0) == -1)) ||
                   ((programFd[1] != -1 ) && (dup2(programFd[1],1) == -1))  ){
                perror("dup2");
                exit(errno);
                
            }
            /*the child now will exit #include <unistd.h>
// execlp args
int execlp( const char * file, 
            const char * arg0, 
            const char * arg1,
            … 
            const char * argn, 
            NULL );*/
            execlp(program,program,NULL);// replaces the current process image with a new process image specified by file - which is the 
            // specified fule name replacement
            exit(errno);
            
            //printf();
        }
        
        
        
    }// exit for loop now
    
    // waiting for child process to complete
    //pid_t waitpid(pid_t pid, int *status, int options);
    for(i = 1; i < argc; i++){
        int wstatus;
        if(waitpid(pids[i-1],&wstatus, 0) == -1){
            perror("wait");
            exit(errno);
        }
        // Checking the exit code of the child is done by WIFEEXITED GEEKSFORGEEKS
        /*WIFEXITED and WEXITSTATUS are two of the options which can be used to know the exit status of the child. 
WIFEXITED(status) : returns true if the child terminated normally.
WEXITSTATUS(status) : returns the exit status of the child. This macro should be employed only if WIFEXITED returned true.*/
        if(WIFEXITED(wstatus) != 0){
            // if this is the first chailed file then wait//check then until it isnt.
            if(rv== EXIT_SUCCESS){ // if the first child failed then 
                rv = WEXITSTATUS(wstatus);
            }
        }
    
    }
    // delete arrat
    free(pids);
    return rv;
    
}




/* EYOLFSON LEC16
 * static void check_error(int ret, const char *message) {
    if (ret != -1) {
        return;
    }
    int err = errno;
    perror(message);
    exit(err);
}

static void parent(int in_pipefd[2], int out_pipefd[2], pid_t child_pid) {
}

static void child(int in_pipefd[2], int out_pipefd[2], const char *program) {
    execlp(program, program, NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return EINVAL;
    }

    int in_pipefd[2] = {0};

    int out_pipefd[2] = {0};

    pid_t pid = fork();
    if (pid > 0) {
        parent(in_pipefd, out_pipefd, pid);
    }
    else {
        child(in_pipefd, out_pipefd, argv[1]);
    }

    return 0;
}
 
 
 
 */


#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/* Linux host fixture: no network/hardware; only its own child processes/tmp file. */
static void wait_exit(pid_t pid,int expected){
    int status=0;pid_t done;
    do {done=waitpid(pid,&status,0);} while(done<0&&errno==EINTR);
    assert(done==pid&&WIFEXITED(status)&&WEXITSTATUS(status)==expected);
}
static void test_fork_exec(void){
    int pipefd[2];assert(pipe(pipefd)==0);
    pid_t child=fork();assert(child>=0);
    if(child==0){
        close(pipefd[0]);const char marker='F';
        if(write(pipefd[1],&marker,1)!=1)_exit(99);
        close(pipefd[1]);_exit(7); /* no stdio flush/complex cleanup after fork */
    }
    close(pipefd[1]);char marker=0;ssize_t n;
    do {n=read(pipefd[0],&marker,1);} while(n<0&&errno==EINTR);
    assert(n==1&&marker=='F');close(pipefd[0]);wait_exit(child,7);
    child=fork();assert(child>=0);
    if(child==0){execl("/proc/self/exe","host-linux-lifecycle","--child-exec",(char*)NULL);_exit(127);}
    wait_exit(child,0);
    errno=0;assert(waitpid(child,NULL,WNOHANG)==-1&&errno==ECHILD); /* already reaped */
}
static void test_fd_description(void){
    char name[]="/tmp/embedded-fd-fixture-XXXXXX";
    int fd=mkstemp(name);assert(fd>=0);
    assert(unlink(name)==0); /* remove fixture name before later assertions */
    errno=0;assert(access(name,F_OK)==-1&&errno==ENOENT);
    const char data[]="abcde";assert(write(fd,data,5)==5);assert(lseek(fd,0,SEEK_SET)==0);
    int copy=dup(fd);assert(copy>=0);
    char a=0,b=0;assert(read(fd,&a,1)==1&&a=='a');assert(read(copy,&b,1)==1&&b=='b');
    assert(lseek(fd,0,SEEK_CUR)==2); /* both fd entries share one open-file description */
    assert(close(fd)==0);char rest[3]={0};assert(read(copy,rest,3)==3&&memcmp(rest,"cde",3)==0);assert(close(copy)==0);
}
int main(int argc,char**argv){
    if(argc==2&&strcmp(argv[1],"--child-exec")==0)return 0;
    test_fork_exec();test_fd_description();puts("Linux host: PASS (fork/exec/wait reaping, dup shared offset, unlink-open lifetime)");return 0;
}

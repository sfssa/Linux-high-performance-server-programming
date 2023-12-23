#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("usage: %s filename\n", basename(argv[0]));
        return 1;
    }

    int fileFd = open(argv[1], O_CREAT | O_RDONLY | O_TRUNC);
    assert(fileFd > 0);

    int pipefd_stdout[2];
    int ret = pipe(pipefd_stdout);
    assert(ret != -1);

    int pipefd_file[2];
    ret = pipe(pipefd_file);
    assert(ret != -1);

    ret = splice(STDIN_FILENO, NULL, pipefd_stdout[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
    assert(ret != -1);

    ret = tee(pipefd_stdout[0], pipefd_file[1], 32768, SPLICE_F_NONBLOCK);
    assert(ret != -1);

    ret = splice(pipefd_file[0], NULL, fileFd, NULL, 32768, SPLICE_F_MOVE | SPLICE_F_MORE);
    // assert(ret != -1);
    if(ret == -1){
        printf("error is: %d\n", errno);
    }

    ret = splice(pipefd_stdout[0], NULL, STDOUT_FILENO, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
    assert(ret != -1);

    close(fileFd);
    close(pipefd_file[0]);
    close(pipefd_file[1]);
    close(pipefd_stdout[0]);
    close(pipefd_stdout[1]);

    return 0;
}

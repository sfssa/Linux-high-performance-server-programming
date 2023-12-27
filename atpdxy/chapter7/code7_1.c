#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    uid_t real_id = getuid();
    uid_t effective_id = geteuid();
    printf("UID IS : %d\nEUID IS : %d\n", real_id, effective_id);
}

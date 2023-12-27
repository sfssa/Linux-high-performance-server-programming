#include <stdio.h>
#include <unistd.h>

// 以root用户启动，切换到另一个普通用户

static bool switchToUser(const uid_t user_id, const gid_t group_id){
    // 待切换的用户是root
    if((user_id == 0) && (group_id == 0)){
        return false;
    }

    // 当前用户是root或者目标用户
    uid_t uid = getuid();
    printf("user is is: %d\n", uid);
    gid_t gid = getgid();
    if(((uid != 0) || (gid != 0)) && ((gid != group_id) || (uid != user_id))){
        return false;
    }
    
    // 不是root，已经是目标用户
    if(uid != 0){
        return true;
    }

    if((setgid(group_id) < 0) || (setuid(user_id) < 0)){
        return false;
    }

    return true;
}

int main(int argc, char* argv[]){
    switchToUser(1000, 1000);
    uid_t cur_uid = getuid();
    printf("user is is: %d\n", cur_uid);
    return 0;
}

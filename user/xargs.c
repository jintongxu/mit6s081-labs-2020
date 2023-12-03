#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

/*
    之前都是写cpp。现在写c，突然发现c的字符串操作是真的麻烦啊！！！
*/

int main(int argc, char* argv[]) {

    char *new_argv[MAXARG]; // 总参数列表
    char *cmd = argv[1];    // 执行的命令
    // printf("%s\n", new_argv[0]);
    char buf;
    char buf_argv[MAXARG][100];

    while(1) {

        for (int i = 1; i < argc; i ++) {
             new_argv[i - 1] = argv[i]; 
        }
        int now_argv_num = argc - 1, buff = 0;
        memset(buf_argv, 0, MAXARG*100);
        int is_read = 0;

        while((is_read = read(0, &buf, 1) > 0)) {
            if (buf == ' ') {
                now_argv_num++;
                buff = 0;
                continue;
            }

            // \n 表示一个执行的多套操作，是完全隔离的操作，而不是一个操作的多个参数
            if (buf == '\n') {
                break;
            }
            buf_argv[now_argv_num][buff++] = buf;
        }

        // 管道符号前面的参数读取完毕就退出
        // 如果遇到 \n break掉了read的循环，但是管道内参数还没有读取完，说明还需要执行上面那一套操作
        if (is_read <= 0) {
            break;
        }

        for (int i = argc - 1; i <= now_argv_num; i ++ ) {
            new_argv[i] = buf_argv[i];
        }
        if (fork() == 0) {
            exec(cmd, new_argv);
            exit(1);
        }

        wait(0);
    }





    exit(0);
}
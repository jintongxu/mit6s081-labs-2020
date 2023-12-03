#include <kernel/types.h>
#include <user/user.h>

int main(int argc, char* argv[]){
	char a[] = {'a'};
	// 两个管道
	// 管道是两个口,一个只读,一个只写,fd[0]是只读,fd[1]是只写
	int fd1[2], fd2[2];

	pipe(fd1);
	pipe(fd2);
	int pid = fork();
	if (pid > 0) {
		// 父进程
		pid = getpid();
		char buff[10];
		// 父进程写入管道1,子进程读取
		close(fd1[0]);
		write(fd1[1], a, 1);
		// 父进程读取管道2中子进程写入的
		close(fd2[1]);
		read(fd2[0], buff, sizeof(buff));
		printf("%d: received pong\n", pid);

	} else if (pid == 0) {
		// children
		// 关闭管道1的写功能,因为子进程对于管道1只读
		close(fd1[1]);
		char buf[10];
		read(fd1[0], buf, sizeof(buf));
	        printf("%d: received ping\n", 0);	
		// 关闭管道2的读,因为是进程是要写入管道2
		close(fd2[0]);
		write(fd2[1], buf, sizeof(buf));

	}
	exit(0);
}
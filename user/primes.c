#include <kernel/types.h>
#include <user/user.h>
#include <stddef.h>

/*
	https://swtch.com/~rsc/thread/ 具体流程见这个网址中间的流程图
*/
void nextManager(int* pipe_arr) {
	int child[2];
	int current_num;	// 记录当前轮最小的素数，通过筛选掉它的倍数进行素数筛
	close(pipe_arr[1]);
	pipe(child);

	int res = read(pipe_arr[0], &current_num, sizeof(int));
	// 当管道所有内容读完后，再次read会返回0。这个数据丢弃
	if (res == 0) {
		exit(0);
	}

	int pid = fork();
	if (pid == 0) {
		nextManager(child);
	} else if (pid > 0 ) {
		close(child[0]);
		printf("prime %d\n", current_num);
		int prime = current_num;

		while(read(pipe_arr[0], &prime, sizeof(int)) != 0 ){
			// 一直读管道读，将不是当前素数的倍数，放入下一层管道筛选
			if (prime % current_num != 0) {
				write(child[1], &prime, sizeof(int));	
			}
		}
		// 记得关闭通道
		close(child[1]);
		wait(NULL);	
	}
	exit(0);

}

int main(int argc, char* argv[]){

	int pip[2];
	pipe(pip);

	int ret = fork();

	if ( ret == 0 ) {
		nextManager(pip);	
	} else {
		close(pip[0]);
		for (int i = 2; i <= 35; i++) {
			write(pip[1], &i, sizeof(int));
		}

		close(pip[1]);
		wait(NULL);	
	}

	exit(0);

}
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char target[512];

// 文件的相关操作，查看 user/ls.c 文件

// 提取出来当前路径的文件名
char* current_file_name(char *path) {
	// 查找 '/'，
	// strchr返回匹配的第一个字符的地址
	char *curr = strchr(path, '/');
	char *next;

	// 找到最后一个 / 地址，+1才是最后的文件名
	while(curr != 0) {
		next = strchr(curr + 1, '/');
		if (next != 0) {
			curr = next;
		} else {
			break;
		}
	}
	return curr + 1;

}

void find(char *current_path) {
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if ((fd = open(current_path, 0)) < 0 ){
		// fprintf(2, "ls: cannot open %s\n", path);
		return;		

	}

	if (fstat(fd, &st) < 0 ) {
		// fprintf(2, "ls: cannot stat %s\n", path);
		return;
	}

	switch(st.type) {
	case T_FILE:{
		// 判断当前文件名和要找的是不是相等
		char* file_name = current_file_name(current_path);
		if (strcmp(file_name, target) == 0) {
			printf("%s\n", current_path);
		}
		close(fd);
		break;
	}
	case T_DIR:
		if (strlen(current_path) + 1 + DIRSIZ + 1 > sizeof buf) {
			break;
		}
		memset(buf, 0, sizeof(buf));

		// 将当前路径复制到buf，并且在最后增加 /
		int current_length = strlen(current_path);
		memcpy(buf, current_path, current_length);
		buf[current_length ] = '/';

		// 将 p 指向 / 后的一个地址
		p = buf + current_length + 1;
		// printf("%s\n", buf);
		while(read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (de.inum == 0 || strcmp(de.name, ".") == 0  || strcmp(de.name, "..") == 0) 
				continue;
			/*
				p指向buf的地址，修改p就是修改buf
				这里将文件名拼接到最后一个 / 后面，组成完整的路径
				每次修改都将最后一个 / 后面的文件名覆盖
			*/
			memcpy(p, de.name, DIRSIZ);		// fs.h 中宏定义了名字的最大长度 DIRSIZ

			// 继续查找
			find(buf);

		}
		close(fd);
		break;

	}

}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("param num error");
	exit(0);
    }

    strcpy(target, argv[2]);
    find(argv[1]);

    exit(0);
}
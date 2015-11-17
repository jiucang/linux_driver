#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void) {
    int ret, fd;
    char buf;

    //block read
    fd = open("/dev/ttyS1", O_RDWR);
    if (fd < 0)
	printf("fd < 0, open file failed. errno: %d\n", fd);
    ret = read(fd, &buf, 1);
    if (ret == 1)
        printf("%c\n", buf);

    return 0;
}

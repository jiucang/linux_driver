#include <stdio.h>
//#include <execinfo.h>

void my_fun(int);
void (*my_fun_p)(int);

void my_fun(int x) {
    printf("%d\n", x);
}

int main(int argc, char *argv[]) {
	//void *fp;
    my_fun(10);
	my_fun_p = &my_fun;
	(*my_fun_p)(20);
	//fp = &my_fun;
	//backtrace_symbols_fd(&fp, 30);
    return 0;
}

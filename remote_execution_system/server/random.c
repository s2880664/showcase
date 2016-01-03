#include <stdio.h>

int main(int argc, char *argv[]){
	printf("hello world\n");
	SOCKET Sock;

	/* WSAStartup etc. */ 
	if (2 == argc)
	{
		Sock = atoi(argv[1]);   // use Sock
	}
	return 0;
}
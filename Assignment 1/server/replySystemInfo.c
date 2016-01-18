#ifdef __unix__
#include <stdio.h>
#include <string.h> // strcmp
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h> // exit, EXIT_FAILURE 
#include <unistd.h> // execl()
#include <sys/utsname.h>

#include "../common/include.h"

int main(int argc, const char * argv[]) // Socket Descriptor is always in argv[0]
{
    int rSocketDescriptor = atoi(argv[0]); // Convert string representation of int to int

	char* result = "";
	char* newLine = "\n";

    // Get System info using uname()
	struct utsname unameData;
	if(uname(&unameData) != 0)// Might check return value here (non-0 = failure)
        exit(2);

    // Get Host name
	char hostname[1024];
    gethostname(hostname, 1024); 

    result = myConcat(result, hostname);
    result = myConcat(result, newLine);
    result = myConcat(result, unameData.sysname);
    result = myConcat(result, newLine);
    result = myConcat(result, unameData.machine);
    result = myConcat(result, newLine);

    printf("Sleeping 7 seconds.\n");
    Sleep(7000);

    printf("Returning:\n%s\n", result);
	send(rSocketDescriptor, result, strlen(result), 0); // Send back host name
	close(rSocketDescriptor); // close socket
    printf("Connection Closed.\n\n");
    exit(0);
}
#endif // __unix__

#ifdef _WIN32

#endif // _WIN32
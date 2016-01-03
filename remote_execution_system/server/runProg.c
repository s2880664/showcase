#include <stdio.h>
#include <string.h> // strcmp
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h> // exit, EXIT_FAILURE 
#include <unistd.h> // execl()
#include <sys/utsname.h>

#include "../common/include.h"

#define BUFSIZE 128

int main(int argc, char * argv[]) 	// Socket Descriptor is always in argv[0], argv[1] progname, argv[2] args
{
    int rSocketDescriptor = atoi(argv[0]); 	// Convert string representation of int to int
	
	printf("argv[0]%s\nargv[1]%s\nargv[2]%s\n", argv[0], argv[1], argv[2]);

	char errorLocation[256];
    char filename[256];
    strcpy(filename, argv[2]);
    char filenameTok[sizeof(filename)];
    strcpy(filenameTok, filename);
    char *runCmd = "./";
    strtok(filenameTok,".");
    runCmd = myConcat(runCmd, argv[1]);
    runCmd = myConcat(runCmd, "/");
	runCmd = myConcat(runCmd, filenameTok);
	puts("hi1");
	puts(runCmd);

	strcpy(errorLocation, argv[1]);
	strcat(errorLocation, "/error_log.txt");
	//strcat(filename, "/");
    //strcat(filename, argv[2]);
    puts("hi2");
    puts(filename);

	//execlp( "gcc", "gcc", filename, "-o", strtok(filenameTok,"."), "2>", "error_log.txt", NULL ); // No such file or directory: 2> ?????

    char *compileCmd = "gcc ";
    compileCmd = myConcat(compileCmd, argv[1]);
    compileCmd = myConcat(compileCmd, "/");
    compileCmd = myConcat(compileCmd, argv[2]);
    compileCmd = myConcat(compileCmd, " -o ");
    compileCmd = myConcat(compileCmd, argv[1]);
    compileCmd = myConcat(compileCmd, "/");
    compileCmd = myConcat(compileCmd, filenameTok);
	compileCmd = myConcat(compileCmd, " 2> ");// 2> is the stderror channel 
	compileCmd = myConcat(compileCmd, errorLocation);

	printf("CMD:%s.\n", compileCmd);

	char buf[BUFSIZE];
	FILE *fp;

	if ((fp = popen(compileCmd, "r")) == NULL) {
		printf("Error opening pipe!\n");
		exit(2);
	}

	while (fgets(buf, BUFSIZE, fp) != NULL) {
        // Do whatever you want here...
        send(rSocketDescriptor, buf, strlen(buf), 0);
		//printf("OUTPUT: %s", buf);
	}

	if(pclose(fp)){
		printf("Command not found or exited with error status\n");
        //get error_log.txt
        //char buf[CHUNK];
		FILE *file;
		size_t nread;

		file = fopen(errorLocation, "r");
		if (file) {
			while ((nread = fread(buf, 1, sizeof buf, file)) > 0){
				//fwrite(buf, 1, nread, stdout);
				send(rSocketDescriptor, buf, strlen(buf), 0);
			}
			if (ferror(file)) {
				perror("Error reading file");
				exit(2);
			}
			fclose(file);
		}
		exit(0);
	}
	else
	{
    	// Run whatever we just compiled
		printf("Run command:%s\n", runCmd);

		char buf2[BUFSIZE];
		FILE *fp2;

		if ((fp2 = popen(runCmd, "r")) == NULL) {
			printf("Error opening pipe!\n");
			exit(2);
		}

		while (fgets(buf2, BUFSIZE, fp2) != NULL) {
	        // Do whatever you want here...
	        send(rSocketDescriptor, buf2, strlen(buf2), 0);
			//printf("OUTPUT: %s", buf2);
		}

		if(pclose(fp2)) {
			printf("Command not found or exited with error status\n");
		}
	}

	//send(rSocketDescriptor, result, strlen(result), 0); // Send back host name
	close(rSocketDescriptor); // close socket
	exit(0);
}
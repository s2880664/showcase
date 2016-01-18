#define PORT 80

#ifdef __unix__
// UNIX Includes
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>     // exit, EXIT_FAILURE 
#include <unistd.h> // execl()
#include <time.h>
#include "../common/include.h"
#include "../common/defines.h"
//#include <sys/time.h>

#include <stdio.h>
#include <string.h> // strcmp

void childProcess(int);	/* child process prototype  */
void parentProcess(pid_t);	/* parent process prototype */

int main(int argc, char *argv[])
{
	printf("Initiating Server\n");
	// Setup socket for Unix
	struct sockaddr_in name;
	int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	name.sin_port = htons(PORT); //-atoi(argv[1])
	bind(socketDescriptor, (struct sockaddr*) &name, sizeof(name));

	if(listen(socketDescriptor, 5) == -1){ //- set amount of concurrent connections
		perror("listen");
		exit(1);
	}				
	
	pid_t pid;
	int rSocketDescriptor; // Return socket descriptor

  // Timing
  struct timeval start, stop;
  double secs = 0;

	for(;;)
	{
		printf("Waiting for connections..\n");

		rSocketDescriptor = accept(socketDescriptor,0,0);	// Wait for incoming connection and Accept incoming connection
		if (rSocketDescriptor == -1) {perror("accept");continue;}

    gettimeofday(&start, NULL);

		pid = fork();
		if (pid == 0){
			close(socketDescriptor); // child doesn't need the listener
			childProcess(rSocketDescriptor);
		}
		else if (pid > 0)
			parentProcess(pid);
		else
			perror("Fork Error");	

    gettimeofday(&stop, NULL);
    secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    printf("Time taken: %f\n",secs);
	}
	return 0;
}

void childProcess(int rSocketDescriptor)
{
	char buf[1024];
	int cc = recv(rSocketDescriptor, buf, 50, 0);
	if(cc == 0)
		exit(0);
	buf[cc] = 0x00; //-Null terminate the string
	printf("CHILD: Message received: %s.\n", buf);

	char *cmdArray[10];
	int i=0;

	cmdArray[i] = strtok(buf," ");
  printf("cmdArray[%i]: %s\n", i, cmdArray[i]);
	while(cmdArray[i]!=NULL)
	{
		cmdArray[++i] = strtok(NULL," ");
    printf("cmdArray[%i]: %s\n", i, cmdArray[i]);
	}

	if (strcmp(cmdArray[0], "sys") == 0)
	{
		char rSocketDescriptorString[33];
		sprintf(rSocketDescriptorString, "%d", rSocketDescriptor);
		execl("replySystemInfo.exe", rSocketDescriptorString, NULL);
		exit(0); // Just incase
	}
	else if (strcmp(cmdArray[0], "put") == 0)
	{
		char rSocketDescriptorString[33];
		sprintf(rSocketDescriptorString, "%d", rSocketDescriptor);
		receiveFile(rSocketDescriptor, cmdArray); // My receiveFile function, found in common folder.
		exit(0); // Just incase
	}
	else if (strcmp(cmdArray[0], "run") == 0)
	{
		char rSocketDescriptorString[33];
		sprintf(rSocketDescriptorString, "%d", rSocketDescriptor);
		execl("runProg.exe", rSocketDescriptorString, cmdArray[1], cmdArray[2], NULL);
		exit(0); // Just incase
	}
  else if (strcmp(cmdArray[0], "list") == 0)
  {
    char rSocketDescriptorString[33];
    sprintf(rSocketDescriptorString, "%d", rSocketDescriptor);
    execl("listProg.exe", rSocketDescriptorString, cmdArray[1], cmdArray[2], NULL);
    exit(0); // Just incase
  }
	else
	{
    // Default if not supported
		char *reply = "Command Not Supported";
		printf("Returning:\n%s\n", reply);
		send(rSocketDescriptor, reply, strlen(reply), 0);
		close(rSocketDescriptor);
		printf("Connection Closed.\n\n");
	}
	exit(0); // Just incase

}

void parentProcess(pid_t pid)
{
	int status;
	pid_t ret = waitpid(pid, &status, WNOHANG);
	//printf("Witpid() return status: %d\n", ret);	
}

#endif // END __unix__

#ifdef _WIN32

#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <Strsafe.h>
#include <wchar.h>
 
#pragma comment(lib,"ws2_32.lib") //Winsock Library
 
int main(int argc , char *argv[])
{
    WSADATA wsa;
    SOCKET s , OrigSock, DuplicateSock;
    struct sockaddr_in server , client;
    int c;
    char *message;
    // Printing messahge
    char buf[1024];
    int cc;
    // _Popen
   	char   psBuffer[128];
   	FILE   *pPipe;

   	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    wchar_t argbuf[256];
 
    printf("\nServer Initialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
     
    printf("Initialised.\n");
     
    //Create a socket
    if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d" , WSAGetLastError());
    }
 
    printf("Socket created.\n");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(80);
     
    //Bind
    if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }
     
    puts("Bind done");
 
    //Listen to incoming connections
    listen(s , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
     
    c = sizeof(struct sockaddr_in);

    

    while((OrigSock = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET)
    {
        puts("Connection accepted");
		cc = recv(OrigSock, buf, 50, 0);
		if(cc == 0)
			exit(0);
		buf[cc] = 0x00; //-Null terminate the string
		printf("CHILD: Message received: %s.\n", buf);

		
		puts("hi1");
      memset(&si,0,sizeof(si));
puts("hi2");
      // 
      // Duplicate the socket OrigSock to create an inheritable copy.
      // 
      if (!DuplicateHandle(GetCurrentProcess(),
            (HANDLE)OrigSock,
            GetCurrentProcess(),
            (HANDLE*)&DuplicateSock,
            0,
            TRUE, // Inheritable
            DUPLICATE_SAME_ACCESS)) {fprintf(stderr,"dup error %d\n",GetLastError()); return -1;}
      // 
      // Spawn the child process.
      // The first command line argument (argv[1]) is the socket handle.
      // 
puts("hi3");
      	//StringCbPrintf(argbuf,"random.exe %d", DuplicateSock);
		swprintf(argbuf,sizeof(argbuf) / sizeof(*argbuf), L"%d", DuplicateSock);
		wprintf(L"[%ls]\n", argbuf);
puts("hi4");
      if (!CreateProcess(L"random.exe", argbuf,NULL,NULL,
               TRUE, // inherit handles
               0,NULL,NULL,&si,&pi) ){
         fprintf(stderr,"createprocess failed %d\n",GetLastError());
         message = "Error Creating new process\n";
        send(OrigSock , message , strlen(message) , 0);
      }
      puts("hi5");
	  // 
      // On Windows 95, the parent needs to wait until the child
      // is done with the duplicated handle before closing it.
      // 
      WaitForSingleObject(pi.hProcess, INFINITE);

   // The duplicated socket handle must be closed by the owner
   // process--the parent. Otherwise, socket handle leakage
   // occurs. On the other hand, closing the handle prematurely
   // would make the duplicated handle invalid in the child. In this
   // sample, we use WaitForSingleObject(pi.hProcess, INFINITE) to
   // wait for the child.
   // 
   closesocket(OrigSock);
   closesocket(DuplicateSock);
        /* Run DIR so that it writes its output to a pipe. Open this
         * pipe with read text attribute so that we can read it 
         * like a text file. 
         */

   if( (pPipe = _popen( "random.exe hello", "rt" )) == NULL )
      exit( 1 );

   /* Read pipe until end of file, or an error occurs. */

   while(fgets(psBuffer, 128, pPipe))
   {
      printf(psBuffer);
   }


   /* Close pipe and print return value of pPipe. */
   if (feof( pPipe))
   {
     printf( "\nProcess returned %d\n", _pclose( pPipe ) );
   }
   else
   {
     printf( "Error: Failed to read the pipe to the end.\n");
   }

        //Reply to the client
        message = "Hello Client , I have received your connection. But I have to go now, bye\n";
        send(OrigSock , message , strlen(message) , 0);
    }
     
    if (OrigSock == INVALID_SOCKET)
    {
        printf("accept failed with error code : %d" , WSAGetLastError());
        return 1;
    }
 
    closesocket(s);
    WSACleanup();
     
    return 0;
}
#endif // END __WIN32


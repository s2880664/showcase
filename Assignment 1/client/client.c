#ifdef __unix__
#include <stdio.h>
#include <string.h> 	// strcmp
#include <sys/socket.h>
#include <sys/types.h>	// TCP types
#include <netdb.h>		// gethostbyname, htons
#include <unistd.h>		// Sleep
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>	
#include <pthread.h> 	// Threads

#include "../common/include.h"
#include "../common/defines.h"

void *printStuff(void *instance1_ptr);
typedef struct myThing
{
	int *fdmax;
	fd_set *read_fds;
	fd_set *master;
} myThing;

int main(int argc, char *argv[])
{
	char* serverAddress = argv[1];
	if(serverAddress == NULL){
		printf("Error: You need to supply server address in program arguments, E.g. 127.0.0.1\n");
		return 0;
	}

	fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

	// clear the master and temp sets
    FD_ZERO(&master);	
    FD_ZERO(&read_fds);

    int j, rv;
    //char buf[1024];
    //int nbytes;

    char selection [COMMANDLENGTH];		// Stores the full command
    char strtokSelection [COMMANDLENGTH];	// Copy of selection to perform strtoke on to get the first token of command

    int threadStarted = 0;		// Boolean

    pthread_t printStuff_thread;//reference to the second thread

    printf("Sending commands to server at: %s\nClient Ready\n", serverAddress);
    while(1)
    {
    	gets(selection);

    	if(strcmp(selection, "") != 0)
    	{
    		if(strcmp(selection, "quit") == 0) return 0;

            //
            // Setup Socket attributes
            //
            struct sockaddr_in server;
            int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
            struct hostent * hp = gethostbyname(serverAddress);

            bcopy(hp->h_addr, &(server.sin_addr.s_addr), hp->h_length); //bcopy(from, to, howmany)

            server.sin_family = AF_INET;    // Set connection to tcp?
            server.sin_port = htons(PORT);  // Set port number

            //
            // Split Command into tokens
	    	//
            strcpy(strtokSelection, selection);

	    	char *cmdArray[20];
	    	int i=0;

	    	cmdArray[i] = strtok(strtokSelection," "); // Get first token of command string

	    	while(cmdArray[i]!=NULL)
	    	{
	    		cmdArray[++i] = strtok(NULL," ");
	    	}
    	
    		if(strcmp(cmdArray[0], "put") == 0)
    		{
                char buf[COMMANDLENGTH];
                char buf2[COMMANDLENGTH];

				connect(socketDescriptor, (struct sockaddr *)&server, sizeof(server));
				
				send(socketDescriptor, selection, COMMANDLENGTH, 0);// Send the whole command (max 64 bytes)
                
                //
                // Check if server is ready to receive file
                //
                if(recv(socketDescriptor, buf, COMMANDLENGTH, 0) <= 0)
                {
                    perror("recv1");
                }
                if (strcmp(buf, CONTINUESIGNAL) == 0)
                {
                    puts("Running sendFile() command...");
                    // Send the file.
                    sendFile(socketDescriptor, cmdArray); // My sendfile function, found in common folder.
                    // Get confimation of successfull write, or error.
                    //if(recv(socketDescriptor, buf2, COMMANDLENGTH, 0) <= 0)
                    //{
                    //    perror("recv2");
                    //}
                    //else
                    //{
                    //    printf("Server Response:\n%s\n", buf2);
                    //    close(socketDescriptor);
                    //}
                    puts("Assuming the sever recevied the file successfully");
                    puts("Closing socket");
                    close(socketDescriptor);
                }
                else
                {// Server has replied with an error, so we need to print it.
                    printf("%s\n", buf);
                    close(socketDescriptor); // Just incase server didn't
                }
			}
			else
			{
				connect(socketDescriptor, (struct sockaddr *)&server, sizeof(server));

				// Send just the command (COMMANDLENGTH bytes)
				send(socketDescriptor, selection, COMMANDLENGTH, 0);
				printf("Sent: %s\n", selection);
				FD_SET(socketDescriptor, &master); // Add Socket  to master list
				fdmax = socketDescriptor; // Max file descripter number = socketdescripter. no idea why

				read_fds = master;	// Maybe not needed
			}
			if(threadStarted == 0)
			{
				// Create myThing to store the pointers that we need so we can send them to the new thread.
				myThing instance1;
				instance1.fdmax = &fdmax;
				instance1.read_fds = &read_fds;
				instance1.master = &master;
	    		/* create a second thread which prints server results as soon as we recv them */
				if(pthread_create(&printStuff_thread, NULL, printStuff, &instance1)) {
					fprintf(stderr, "Error creating thread\n");
					return 1;
				}
				threadStarted = 1;
			}
		}
	}
	return 0;
}

// Thread that loops constantly and prints results from the server.
void *printStuff(void *instance1_ptr)
{
    fd_set *read_fds2;  // temp file descriptor list for select()
    fd_set *master2; 
    int *fdmax2;
    int i, nbytes;
    char buf[1024];

    struct timeval timeout;

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    read_fds2 = ((myThing*)instance1_ptr)->read_fds;
    master2 = ((myThing*)instance1_ptr)->master;
    fdmax2 = ((myThing*)instance1_ptr)->fdmax;

    while(1)
    {
    	*read_fds2 = *master2;
    	//printf("fdmax2:%i\n", (*fdmax2+1));
    	//if (select(*fdmax2+1, master2, NULL, NULL, &timeout) == -1) // Doesn't work???
    	//{
		//	perror("select");
		//	return NULL;
		//}	
		// run through the existing connections looking for data to read
    	for(i = 0; i <= *fdmax2; i++) 
    	{
    		//printf("i = %d\n", i);
    		if (FD_ISSET(i, master2)) // Was read_fds2
	        { 
	        // we got one!!
	            // handle data from a client
	        	if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) 
	        	{
	                // got error or connection closed by client
	        		if (nbytes == 0) 
	        		{
	                    // connection closed
	        			printf("Server: socket %d hung up\n", i);
	        		} 
	        		else 
	        		{
	        			perror("recv");
	        		}
	                    close(i);
	                    FD_CLR(i, master2); // remove from master set
	                } 
	                else 
	                {
                    // we got some data from a client
                	buf[nbytes] = 0x00; //-Null terminate the string
                	printf("Server Response: \n%s\n\n", buf);
					//shutdown(socketDescriptor,0);
					close(i); // bye!
                    FD_CLR(i, master2); // remove from master set
                }
            } // END handle data from client
        } // END got new incoming connection
    // END looping through file descriptors
		Sleep(100);
    }
    /* the function must return something - NULL will do */
    return NULL;
}
#endif // __unix__

#ifdef _WIN32
/*
    Create a TCP socket
*/
 
#include <stdio.h>
#include <winsock2.h>

#include "../common/defines.h"
 
#pragma comment(lib,"ws2_32.lib") //Winsock Library
 
int main(int argc , char *argv[])
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char *server_reply[2000], message[COMMANDLENGTH];
    //char *message[COMMANDLENGTH];
    int recv_size;
 
    printf("\nInitialising Winsock...");
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
     
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
 
    //Connect to remote server
    if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }
     
    puts("Connected");
     
    //Send some data
    gets(message);
    //message = "GET / HTTP/1.1\r\n\r\n";
    if(send(s, message, COMMANDLENGTH, 0) < 0)
    {
        puts("Send failed");
        return 1;
    }
    puts("Data Send\n");
     
    //Receive a reply from the server
    if((recv_size = recv(s , server_reply , 2000 , 0)) == SOCKET_ERROR)
    {
        puts("recv failed");
    }
     
    puts("Reply received\n");
 
    //Add a NULL terminating character to make it a proper string before printing
    server_reply[recv_size] = '\0';
    puts(server_reply);
 
 	closesocket(s);
	WSACleanup();

    return 0;
}
#endif _WIN32
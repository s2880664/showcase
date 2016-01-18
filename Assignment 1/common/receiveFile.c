#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> // mkdir
#include <dirent.h>   //Directory stuff

#include "defines.h"

void receiveFile(int rSocketDescriptor, char *cmdArray[]) //-argv[0] Socket Descriptor, argv[1] fileName directory, argv[2] -f flag (optional) 
{
    char *fileNames[10];
    int fFlagBool = 0;

    int i = 0;

    // Check for '-f' flag
    while(cmdArray[i] != NULL)
    {
        if(strcmp(cmdArray[i], "-f") == 0) fFlagBool = 1;
        i++;
    }

    
    //
    // Create Folder
    //
    struct stat sb;
    if(!(stat(cmdArray[1], &sb) == 0 && S_ISDIR(sb.st_mode))){ // Folder does not exist
        if (mkdir(cmdArray[1], 0777) == -1) {
            perror("mkdir");
            exit(2);
        }
    }
    else if((stat(cmdArray[1], &sb) == 0 && S_ISDIR(sb.st_mode)) && fFlagBool == 1)
    {
        // These are data types defined in the "dirent" header
        DIR *theFolder = opendir(cmdArray[1]);
        struct dirent *next_file;
        char filepath[256];

        while ((next_file = readdir(theFolder)) != NULL)
        {
            // build the path for each file in the folder
            sprintf(filepath, "%s/%s", cmdArray[1], next_file->d_name);
            printf("Deleting file: %s\n", filepath);
            remove(filepath);
        }
    }
    else
    {
        char *reply = "Error: Progname all ready exits.";
        printf("Replying: %s\n", reply);
        send(rSocketDescriptor, reply, COMMANDLENGTH, 0); // Send back host name
        close(rSocketDescriptor); // close socket
        exit(0); 
    }

    //Send signal to say yes to file transfer
    send(rSocketDescriptor, CONTINUESIGNAL, CONTINUESIGNAL_LENGTH, 0);
    //
    // Transfer file
    //
    int bytesReceived = 0;
    char recvBuff[256];
    memset(recvBuff, '0', sizeof(recvBuff));
    puts(cmdArray[1]);
    puts(cmdArray[2]);

    char str[80];
    strcat(str, cmdArray[1]);
    strcat(str, "\\");
    strcat(str, cmdArray[2]);

    //long int fileLength;
    //char fileLengthBuf[COMMANDLENGTH];
    //
    // Get length of file to recv
    //
    //printf("1 File length buffer: %s\nFile length: %d\n", fileLengthBuf, fileLength);
    //if(recv(rSocketDescriptor, fileLengthBuf, COMMANDLENGTH, 0) <= 0)
    //{
    //    perror("recv3");
    //}

    //fileLength = strtol(fileLengthBuf,NULL,0);

    //printf("File length buffer: %s\nFile length: %d\n", fileLengthBuf, fileLength);

    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen(str, "w+b"); 
    if(NULL == fp)
    {
        printf("Error opening file");
        exit(2);
    }

    /* Receive data in chunks of 256 bytes */
    while((bytesReceived = read(rSocketDescriptor, recvBuff, 256)) > 0)
    {
        printf("Bytes received %d\n",bytesReceived);    
        fwrite((recvBuff+14), 1,(bytesReceived-14),fp);
    }

    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }

    fflush(fp);

    puts("Flushed file to disk");

    
    //send(rSocketDescriptor, "File transfer completed successfully!", COMMANDLENGTH, 0); // Send back host name
    close(rSocketDescriptor); // close socket
    exit(0);
}

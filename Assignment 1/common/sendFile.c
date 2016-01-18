#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "defines.h"

void sendFile(int rSocketDescriptor, char *cmdArray[]) //-argv[0] Socket Descriptor, cmdArray[1] progname directory, argv[2] -f flag (optional)
{
    printf("Sending stuff!\n");

    //long int fileLength;
    //char fileLengthBuf[COMMANDLENGTH];

    char sendBuff[1025];
    memset(sendBuff, '0', sizeof(sendBuff));

    /* Open the file that we wish to transfer */
    FILE *fp = fopen(cmdArray[2], "r");
    if(fp==NULL)
    {
        printf("File open error");
        exit(0);   
    }   

    //fseek(fp, 0, SEEK_END);  // seek to end of file
    //fileLength = ftell(fp);        // get current file pointer
    //fseek(fp, 0, SEEK_SET);  // seek back to beginning of file

    //sprintf(fileLengthBuf, "%d", fileLength);
    //printf("File lenght Buffer: %s  COMMANDLENGTH:%d\n", fileLengthBuf, COMMANDLENGTH);

    //send(rSocketDescriptor, fileLengthBuf, COMMANDLENGTH, 0);

    
    /* Read data from file and send it */
    while(1)
    {
        /* First read file in chunks of 256 bytes */
        unsigned char buff[256]={0};
        int nread = fread(buff,1,256,fp);
        printf("Bytes read %d \n", nread);        

        /* If read was success, send data. */
        if(nread > 0)
        {
            printf("Sending \n");
            write(rSocketDescriptor, buff, nread);
        }

        /*
         * There is something tricky going on with read .. 
         * Either there was error, or we reached end of file.
         */
        if (nread < 256)
        {
            if (feof(fp))
                printf("End of file\n");
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
    close(rSocketDescriptor);
    //exit(0);
}

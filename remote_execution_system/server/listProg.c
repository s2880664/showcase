#include <stdio.h>
#include <string.h> // strcmp
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h> // exit, EXIT_FAILURE 
#include <sys/utsname.h>
#include <sys/stat.h> // ISDIR
#include <dirent.h>   //Directory stuff

#include "../common/include.h"

int EndsWith(const char *str, const char *suffix);

int main(int argc, char * argv[])  //-argv[0] Socket Descriptor, cmdArray[1] progname directory, argv[2] -f flag (optional)
{
	int rSocketDescriptor = atoi(argv[0]); 	// Convert string representation of int to int

	puts("starting listProg");
	printf("argv[0]%s\nargv[1]%s\nargv[2]%s\n", argv[0], argv[1], argv[2]);

	DIR *d;
	struct dirent *dir;
	struct stat statbuf;

	char resultBuf[1024];
	char currentPath[FILENAME_MAX];

	char date[20];

	if(argv[1] != NULL && strcmp(argv[1], "-l") == 0)
	{
		if (argv[2] == NULL)
		{
			//
			// Long list directory
			//

			// Store Current Working Directory in currentPath
  			if((getcwd(currentPath, FILENAME_MAX)) == NULL)
    			return errno;

			if ((d = opendir(currentPath)) == NULL)
  				perror("opendir Error");
  			while((dir = readdir(d)) != NULL)
  			{
			   	/*Skips . and ..*/
			    if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			    	continue;
				
				if(stat(dir->d_name, &statbuf) == -1){
			    	perror("stat");
			    	return errno;
			    }
			    if(S_ISDIR(statbuf.st_mode))
			   	{
			   		strftime(date, 20, "%d-%m-%y", localtime(&(statbuf.st_ctime)));
			   		sprintf(resultBuf + strlen(resultBuf), "%s\n", dir->d_name);//st_size
					sprintf(resultBuf + strlen(resultBuf), "File size: %d bytes.\n", statbuf.st_size);
					sprintf(resultBuf + strlen(resultBuf), "Creation Time:  %s.\n", date);
			   		sprintf(resultBuf + strlen(resultBuf),(S_ISDIR(statbuf.st_mode)) ? "d" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IRUSR) ? "r" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IWUSR) ? "w" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IXUSR) ? "x" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IRGRP) ? "r" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IWGRP) ? "w" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IXGRP) ? "x" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IROTH) ? "r" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IWOTH) ? "w" : "-");
					sprintf(resultBuf + strlen(resultBuf),(statbuf.st_mode & S_IXOTH) ? "x" : "-");
					sprintf(resultBuf + strlen(resultBuf),"\n\n");
			   		printf("Adding %s to list\n", dir->d_name);
			   		
				   	//strcat(resultBuf, dir->d_name);
				   	//strcat(resultBuf, "\n");
				}
  			}
  			closedir(d);
		}
		else
		{
			//
			// Long list files in progname argv[2]
			//
			
		}
	}
	else
	{
		if (argv[1] == NULL)
		{
			//
			// Normal list directory
			//
			// Store Current Working Directory in currentPath
  			if((getcwd(currentPath, FILENAME_MAX)) == NULL)
    			return errno;

			printf("argv[1]%s\nargv[2]%s\n", argv[1], argv[2]);
			if ((d = opendir(currentPath)) == NULL)
  				perror("opendir Error");
  			while((dir = readdir(d)) != NULL)
  			{
			   	/*Skips . and ..*/
			    if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			    	continue;
				
				if(stat(dir->d_name, &statbuf) == -1){
			    	perror("stat");
			    	return errno;
			    }
			    if(S_ISDIR(statbuf.st_mode))
			   	{
			   		printf("Adding %s to resultBuf\n", dir->d_name);
			   		sprintf(resultBuf + strlen(resultBuf), "%s\n", dir->d_name);
				   	//strcat(resultBuf, dir->d_name);
				   	//strcat(resultBuf, "\n");
				}
  			}
  			closedir(d);
		}
		else
		{
			//
			// Normal list files in progname argv[1]
			//
			if((stat(argv[1], &statbuf) == 0 && S_ISDIR(statbuf.st_mode)))
    		{
		        // These are data types defined in the "dirent" header
		        d = opendir(argv[1]);

		        while ((dir = readdir(d)) != NULL)
		        {
		        	/*Skips . and ..*/
				    if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
				    	continue;
		            // build the path for each file in the folder
		            printf("Adding %s to resultBuf\n", dir->d_name);
		            sprintf(resultBuf + strlen(resultBuf), "%s\n", dir->d_name);
		        }
		    }
		    else
		    {
		        char *reply = "Error: Progname does not exist.";
		        printf("Replying: %s\n", reply);
		        send(rSocketDescriptor, reply, COMMANDLENGTH, 0); // Send back host name
		    }
		}
	}
	printf("Returning result:\n%s\n", resultBuf);
  	send(rSocketDescriptor, (resultBuf+1), strlen(resultBuf), 0); // +1 to get rid of the 'm' at the start of the buffer for some reason.
	close(rSocketDescriptor);
	exit(0);
}

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}
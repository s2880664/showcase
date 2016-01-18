#pragma once

#include "sendFile.c"
#include "receiveFile.c"
#include "utilities.c"

//prototypes
void sendFile(int rSocketDescriptor, char *cmdArray[]);

void receiveFile(int rSocketDescriptor, char *cmdArray[]);

char* myConcat(char *s1, char *s2);
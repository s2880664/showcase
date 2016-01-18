OBJECTS = server.o
OBJECTS2 = listProg.o
OBJECTS3 = runProg.o
OBJECTS4 = replySystemInfo.o

all: server listProg runProg replySystemInfo

server: server.o
	gcc $(OBJECTS) -o server
listProg: listProg.o	
	gcc $(OBJECTS2) -o listProg 
runProg: runProg.o	
	gcc $(OBJECTS3) -o runProg
replySystemInfo: replySystemInfo.o	
	gcc $(OBJECTS4) -o replySystemInfo 
	
	
server.o: server.c
	gcc -c server.c
listProg.o: listProg.c
	gcc -c listProg.c
runProg.o: runProg.c
	gcc -c runProg.c
replySystemInfo.o: replySystemInfo.c
	gcc -c replySystemInfo.c
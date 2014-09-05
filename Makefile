CC = gcc
OBJ = tftpd.o packetopt.o translate.o handlecmd.o
test : $(OBJ)
	$(CC) $^ -o $@ -g -Wall -pthread 
%.o : %.c
	$(CC)  -c  $^
clean:
	rm *.o test

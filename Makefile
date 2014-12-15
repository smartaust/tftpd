CC := gcc
OBJS := $(wildcard *.c)
OBJ := $(OBJS:%.c=%.o)
test : $(OBJ)
	$(CC) $^ -o $@ -g -Wall -pthread 
%.o : %.c
	$(CC)  -c  $^  
.PHONY : clean
clean:
	-rm *.o test

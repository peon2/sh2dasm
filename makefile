OBJS		= sh2dasm.o opcodes.o cps3dec.o argp.o
CFLAGS		= -g -Wall


sh2dasm :	$(OBJS)
	$(CC) -o sh2dasm $(OBJS)

sh2dasm.o :	sh2dasm.h 

opcodes.o :	sh2dasm.h opcodes.h

cps3dec.o :	sh2dasm.h cps3dec.h

argp.o :	sh2dasm.h

clean :
		-rm -f $(OBJS) sh2dasm

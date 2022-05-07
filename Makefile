CC      = gcc -g
# CFLAGS  = -Wall -Werror -std=gnu17 -ggdb3 -Og
CFLAGS =
LDFLAGS =
PRG     = transport

OBJFILE =  Output.o  Main.o  Input.o Socket.o Manager.o Repository.o

${PRG}: ${OBJFILE}
	${CC} ${OBJFILE} ${LDFLAGS} -o $@

%.o: %.c *.h
	${CC} $<  -c ${CFLAGS} -o $@

.PHONY: clean  run

distclean:
	rm -f ${PRG}
	rm -f ${OBJFILE} 

clean:
	rm -f ${OBJFILE}
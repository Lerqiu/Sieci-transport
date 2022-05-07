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

.PHONY: clean  run check

distclean:
	rm -f ${PRG}
	rm -f ${OBJFILE} 

clean:
	rm -f ${OBJFILE}

check:
	valgrind --leak-check=full \
			--show-leak-kinds=all \
			--track-origins=yes \
			--verbose \
			--log-file=valgrind-out.txt \
			./transport 127.0.0.1 50001 output 15123
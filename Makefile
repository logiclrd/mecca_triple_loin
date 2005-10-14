CCOPTS = -O3

default: mtl libmtl
	@printf "\n\n"
	@ls -l mtl libmtl.a
	@printf "\n\n"

install: mtl libmtl
	@echo Install it yourself you lazy bone.

clean:
	rm -f main.o program.o parser.o interpret.o io.o fuckup.o

distclean: clean
	rm -f mtl

.PHONY: default install clean distclean

mtl: main.o program.o parser.o interpret.o io.o fuckup.o
	gcc -o mtl main.o program.o parser.o interpret.o io.o fuckup.o
	strip mtl

libmtl: fuckup.o io.o
	ar -r libmtl.a fuckup.o io.o

main.o: main.c interpret.h parser.h types.h
	gcc -c -o main.o main.c ${CCOPTS}

program.o: program.c program.h fuckup.h types.h
	gcc -c -o program.o program.c ${CCOPTS}

parser.o: parser.c program.h fuckup.h types.h syslib.i.h
	gcc -c -o parser.o parser.c ${CCOPTS}

interpret.o: interpret.c interpret.h program.h fuckup.h types.h io.h
	gcc -c -o interpret.o interpret.c ${CCOPTS}

io.o: io.c fuckup.h types.h
	gcc -c -o io.o io.c ${CCOPTS}

fuckup.o: fuckup.c fuckup.h types.h
	gcc -c -o fuckup.o fuckup.c ${CCOPTS}

program.h: types.h

parser.h: program.h

interpret.h: program.h

fuckup.h: types.h


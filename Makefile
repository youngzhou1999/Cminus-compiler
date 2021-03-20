CC = gcc
CFLAGS = -Wall
LDFLAGS = -ll -ly

LEX = flex
LFLAGS =
YACC = bison
YFLAGS = -d

SRC = main.c scan.c parse.c AST.c symtab.c cgen.c code.c


all: main tm

main: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ -g
tm: tm.c
	$(CC) $(CFLAGS) $< -o $@


## For Lex and Yacc
scan.c: cminus.l globals.h parse.h 
	$(LEX) $(LFLAGS) -o $@ $< 


parse.c parse.h: cminus.y globals.h AST.h
	$(YACC) $(YFLAGS) -o $@ $< 

##


clean:
	rm -f *.o 
	rm -f scan.c
	rm -f parse.c
	rm -f parse.h
	rm -f main
	rm -f tm


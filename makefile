all: rez
rez: main.c
	gcc main.c -o rez -lpanel -lncurses
main.c:

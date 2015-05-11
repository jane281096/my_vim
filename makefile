all: my_vim
my_vim: main.c
	gcc main.c -o my_vim -lpanel -lncurses
main.c:

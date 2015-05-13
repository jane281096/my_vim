all: cute_vim
cute_vim: main.c
	gcc main.c -o cute_vim -lpanel -lncurses
main.c:

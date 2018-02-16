mk: t.c s.s queue.o
	gcc -g -m32 -o run t.c s.s queue.o

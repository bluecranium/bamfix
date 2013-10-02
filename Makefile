make: bamfix.c
	gcc -g -o bamfix bamfix.c -Isamtools -lbam -Lsamtools -lz -lm -lpthread

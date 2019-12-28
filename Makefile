%: %.c
	gcc -o $@ $@.c -lm -Wall

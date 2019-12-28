%: %.c
	gcc -o $@ $@.c -lm -Wall
	cpplint --filter=-legal/copyright $@.c

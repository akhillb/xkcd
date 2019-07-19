CC=gcc
LIBS=-lcurl -ljson-c

xkcd:
	$(CC) -o bin/xkcd src/main.c $(LIBS)

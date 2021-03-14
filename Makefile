CC := clang
CFLAGS := -g -Wall -Werror

all: mysh

clean:
	rm -rf mysh

mysh: mysh.c
	$(CC) $(CFLAGS) -o mysh mysh.c

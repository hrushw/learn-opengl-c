CC = gcc -Wall -Wextra -Wpedantic -Wvla
LFLAGS = -lglfw -lm -lepoxy

render: render.c
	$(CC) render.c $(LFLAGS) -o render

clean:
	rm -f render

all: render

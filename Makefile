CC = gcc -Wall -Wextra -Wvla -Wpedantic
INC = -I ./include

render: render.o
	$(CC) -lglfw -lm -lepoxy render.o -o render

render.o: render.c
	$(CC) render.c $(INC) -c -o render.o

clean:
	rm -f *.o render

all: render

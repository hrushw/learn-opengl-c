CC = gcc -Wall -Wextra -Wvla -Wpedantic
INC = -I ./include

render: obj/render.o
	$(CC) -lglfw -lm -lepoxy obj/render.o -o render

obj/render.o: src/render.c
	$(CC) src/render.c $(INC) -c -o obj/render.o

clean:
	rm -f obj/*.o render

all: render

CC = gcc -Wall -Wextra -Wvla -Wpedantic
INC = -I ./include

render: obj/render.o obj/gl.o
	$(CC) -lglfw obj/render.o obj/gl.o -o render

obj/render.o: src/render.c
	$(CC) src/render.c $(INC) -c -o obj/render.o

obj/gl.o: src/gl.c
	$(CC) src/gl.c $(INC) -c -o obj/gl.o

clean:
	rm -f obj/*.o render

all: render

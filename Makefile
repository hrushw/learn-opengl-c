CC = gcc -Wall -Wextra -Wvla

render: obj/render.o obj/gl.o
	$(CC) -lm -lglfw obj/render.o obj/gl.o -o render

obj/render.o: src/render.c
	$(CC) src/render.c -I ./include -c -o obj/render.o

obj/gl.o: src/gl.c
	$(CC) src/gl.c -I ./include -c -o obj/gl.o

clean:
	rm -f obj/*.o render

all: render

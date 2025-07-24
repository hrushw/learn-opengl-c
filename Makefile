CC = gcc -Wall -Wextra -Wpedantic -Wvla
LFLAGS = -lglfw -lm -lepoxy

render: obj/render.o obj/window.o
	$(CC) obj/render.o obj/window.o $(LFLAGS) -o render

obj/window.o: src/window.c src/window.h
	$(CC) src/window.c -c -o obj/window.o

obj/render.o: src/render.c src/window.h
	$(CC) src/render.c -c -o obj/render.o

clean:
	rm -f render obj/*.o

all: render

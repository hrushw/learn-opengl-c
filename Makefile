CC = gcc -Wall -Wextra -Wpedantic -Wvla
LFLAGS = -lglfw -lm -lepoxy

render: obj/render.o obj/window.o obj/main.o
	$(CC) obj/render.o obj/window.o obj/main.o $(LFLAGS) -o render

obj/main.o: src/main.c src/window.h
	$(CC) src/main.c -c -o obj/main.o

obj/window.o: src/window.c src/window.h
	$(CC) src/window.c -c -o obj/window.o

obj/render.o: src/render.c src/vector.c src/window.h
	$(CC) src/render.c -c -o obj/render.o

clean:
	rm -f render obj/*.o

all: render

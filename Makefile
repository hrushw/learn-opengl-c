CC = gcc -Wall -Wextra -Wpedantic -Wvla -I ./include
LFLAGS = -lglfw -lm -lepoxy

render: obj/vector.o obj/render.o obj/window.o obj/main.o
	$(CC) obj/vector.o obj/render.o obj/window.o obj/main.o $(LFLAGS) -o render

obj/main.o: src/main.c include/window.h
	$(CC) src/main.c -c -o obj/main.o

obj/window.o: src/window.c include/window.h
	$(CC) src/window.c -c -o obj/window.o

obj/render.o: src/render.c include/window.h
	$(CC) src/render.c -c -o obj/render.o

obj/vector.o: src/vector.c include/vector.h
	$(CC) src/vector.c -c -o obj/vector.o

clean:
	rm -f render obj/*.o

all: render

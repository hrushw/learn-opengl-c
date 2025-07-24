CC = gcc -Wall -Wextra -Wpedantic -Wvla
LFLAGS = -lglfw -lm -lepoxy
INCFLAGS = -I ./include
OBJS = obj/main.o obj/matrix.o obj/shader.o obj/render.o obj/window.o

all: render

render: $(OBJS)
	$(CC) $(OBJS) $(INCFLAGS) $(LFLAGS) -o render

obj/main.o: src/main.c include/window.h
	$(CC) src/main.c $(INCFLAGS) -c -o obj/main.o

obj/window.o: src/window.c include/window.h
	$(CC) src/window.c $(INCFLAGS) -c -o obj/window.o

obj/render.o: src/render.c include/window.h include/matrix.h
	$(CC) src/render.c $(INCFLAGS) -c -o obj/render.o

obj/matrix.o: src/matrix.c include/matrix.h
	$(CC) src/matrix.c $(INCFLAGS) -c -o obj/matrix.o

obj/shader.o: src/shader.c
	$(CC) src/shader.c $(INCFLAGS) -c -o obj/shader.o

clean:
	rm -f render obj/*.o

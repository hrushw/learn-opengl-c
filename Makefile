CC = gcc -Wall -Wextra -Wpedantic -Wvla
LFLAGS = -lglfw -lm -lepoxy
INCFLAGS = -I ./include
OBJS = obj/main.o obj/render.o obj/window.o

all: render

render: $(OBJS)
	$(CC) $(OBJS) $(INCFLAGS) $(LFLAGS) -o render

obj/main.o: src/main.c include/window.h
	$(CC) src/main.c $(INCFLAGS) -c -o obj/main.o

obj/window.o: src/window.c include/window.h
	$(CC) src/window.c $(INCFLAGS) -c -o obj/window.o

obj/render.o: src/render.c include/window.h
	$(CC) src/render.c $(INCFLAGS) -c -o obj/render.o

clean:
	rm -f render obj/*.o

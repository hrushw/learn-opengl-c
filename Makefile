render: obj/render.o obj/gl.o
	gcc -Wall -Wextra -lm -lglfw obj/render.o obj/gl.o -o render

obj/render.o: src/render.c
	gcc -Wall -Wextra src/render.c -I ./include -c -o obj/render.o

obj/gl.o: src/gl.c
	gcc -Wall -Wextra src/gl.c -I ./include -c -o obj/gl.o

clean:
	rm -f obj/*.o render

all: render

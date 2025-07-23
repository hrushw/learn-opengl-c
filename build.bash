#!/bin/bash

CC="gcc -Wall -Wextra -Wpedantic -Wvla"
LFLAGS="-lglfw -lm -lepoxy"

cmdshow() {
	echo $*
	eval $@
}

main() {
	window
	render

	[[ src/main.c -nt render || include/window.h -nt render ]] &&
		cmdshow "$CC src/main.c obj/render.o obj/window.o -I ./include $LFLAGS -o render"
}

render() {
	[[ src/render.c -nt render || include/window.h -nt render ]] &&
		cmdshow "$CC src/render.c $LFLAGS -I ./include -c -o obj/render.o"
}

window() {
	[[ src/window.c -nt render || include/window.h -nt render ]] &&
		cmdshow "$CC src/window.c $LFLAGS -I ./include -c -o obj/window.o"
}

clean() {
	cmdshow "rm render"
}

case $1 in
	"")
		main;;
	"clean")
		clean;;
esac

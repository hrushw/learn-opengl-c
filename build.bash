#!/bin/bash

CC="gcc -Wall -Wextra -Wpedantic -Wvla"
LFLAGS="-lglfw -lm -lepoxy"

cmdshow() {
	echo "$@"
	eval $@
}

checkrun() {
	eval "$1" && cmdshow ${@:2}
}


main_test() {
	[[ src/main.c -nt render || include/window.h -nt render ]]
}
main() {
	window

	matrix
	render

	checkrun main_test $CC src/main.c obj/window.o obj/render.o obj/matrix.o -I ./include $LFLAGS -o render
}

window_test() {
	[[ src/window.c -nt render || include/window.h -nt render ]]
}
window() {
	checkrun window_test $CC src/window.c -I ./include -c -o obj/window.o
}


matrix_test() {
	[[ src/matrix.c -nt render || include/matrix.h -nt render ]]
}
matrix() {
	checkrun matrix_test $CC src/matrix.c -I ./include -c -o obj/matrix.o
}


render_test() {
	[[ src/render.c -nt render || include/window.h -nt render ]]
}
render() {
	checkrun render_test $CC src/render.c -I ./include -c -o obj/render.o
}


clean_test() {
	[[ -e render ]]
}
clean() {
	checkrun clean_test rm render
}


case $1 in
	"" | "all")
		main;;
	"render")
		render;;
	"window")
		window;;
	"clean")
		clean;;
	*)
		echo "Invalid Target"
esac

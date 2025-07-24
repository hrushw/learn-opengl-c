#!/bin/bash

CC="gcc -Wall -Wextra -Wpedantic -Wvla"
LFLAGS="-lglfw -lm -lepoxy"
INCFLAGS="-I ./include"

cmdshow() {
	echo "$@"
	eval $@
}

checkrun() {
	eval "$1" || [ build.bash -nt render ] && cmdshow ${@:2}
}


main_test() {
	[[ src/main.c -nt render || include/window.h -nt render ]] || window_test || render_test || matrix_test
}
main() {
	window

	matrix
	shader
	render

	checkrun main_test $CC src/main.c obj/window.o obj/render.o obj/shader.o obj/matrix.o $INCFLAGS $LFLAGS -o render
}

window_test() {
	[[ src/window.c -nt render || include/window.h -nt render ]]
}
window() {
	checkrun window_test $CC src/window.c $INCFLAGS -c -o obj/window.o
}


matrix_test() {
	[[ src/matrix.c -nt render || include/matrix.h -nt render ]]
}
matrix() {
	checkrun matrix_test $CC src/matrix.c $INCFLAGS -c -o obj/matrix.o
}

shader_test() {
	[ src/shader.c -nt render ]
}
shader() {
	checkrun shader_test $CC src/shader.c $INCFLAGS -c -o obj/shader.o
}

render_test() {
	[[ src/render.c -nt render || include/window.h -nt render ]]
}
render() {
	checkrun render_test $CC src/render.c $INCFLAGS -c -o obj/render.o
}


clean_test() {
	[[ -e render || -e obj/*.o ]]
}
clean() {
	checkrun clean_test rm render obj/*.o
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


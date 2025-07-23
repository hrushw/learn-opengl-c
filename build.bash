#!/bin/bash

CC="gcc -Wall -Wextra -Wpedantic -Wvla"
LFLAGS="-lglfw -lm -lepoxy"

cmdshow() {
	echo "$1"
	eval "$1"
}

checkrun() {
	eval "$1" && cmdshow "$2"
}

main_test() {
	[[ src/main.c -nt render || include/window.h -nt render ]]
}
main() {
	window
	render

	checkrun main_test "$CC src/main.c obj/render.o obj/window.o -I ./include $LFLAGS -o render"
}

render_test() {
	[[ src/render.c -nt render || include/window.h -nt render ]]
}
render() {
	checkrun render_test "$CC src/render.c $LFLAGS -I ./include -c -o obj/render.o"
}

window_test() {
	[[ src/window.c -nt render || include/window.h -nt render ]]
}
window() {
	checkrun window_test "$CC src/window.c $LFLAGS -I ./include -c -o obj/window.o"
}

clean_test() {
	[[ -e render ]]
}
clean() {
	checkrun clean_test "rm render"
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

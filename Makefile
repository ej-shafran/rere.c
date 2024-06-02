build/rere: src/rere.c src/bi.h
	mkdir -p build/
	gcc -ggdb -o build/rere src/rere.c

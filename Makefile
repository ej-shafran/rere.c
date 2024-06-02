build/rere: src/rere.c src/bi.h
	mkdir -p build/
	gcc -ggdb -o build/rere src/rere.c

.PHONY: test
test: build/rere test/*.list
	./build/rere replay ./test/*.list

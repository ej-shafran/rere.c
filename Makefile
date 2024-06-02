build/rere: src/rere.c src/bi.h
	mkdir -p build/
	gcc -Wall -Wextra -Werror -pedantic -O3 -o build/rere src/rere.c

build/rere-debug: src/rere.c src/bi.h
	mkdir -p build/
	gcc -Wall -Wextra -Werror -pedantic -ggdb -o build/rere-debug src/rere.c

.PHONY: debug
debug: build/rere-debug

.PHONY: test
test: build/rere test/*.list
	./build/rere replay ./test/*.list

.PHONY: record
record: build/rere test/*.list
	./build/rere record ./test/*.list

compile_flags = -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf -Wall -Wextra -pedantic -std=c99
pong: main.c
	rm -rf build
	mkdir build
	$(CC) main.c -o build/pong $(compile_flags)
debug: main.c
	rm -rf debug
	mkdir debug
	$(CC) main.c -o debug/pong $(compile_flags)
clean:
	rm -rf build
	rm -rf debug

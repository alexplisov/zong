pong: main.c
	rm -rf build
	mkdir build
	$(CC) main.c -o build/pong -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -Wall -Wextra -pedantic -std=c99
debug: main.c
	rm -rf debug
	mkdir debug
	$(CC) -g main.c -o debug/pong -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -Wall -Wextra -pedantic -std=c99
clean:
	rm -rf build
	rm -rf debug

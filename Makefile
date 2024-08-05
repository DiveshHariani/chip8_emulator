CC = clang
IDIR_SDL = /opt/homebrew/opt/sdl2/include/SDL2
IDIR_SDL_IMG = /opt/homebrew/opt/sdl2_image/include/SDL2
LDIR_SDL = /opt/homebrew/opt/sdl2/lib
LDIR_SDL_IMG = /opt/homebrew/opt/sdl2_image/lib

LLIB = -lSDL2 -lSDL2_image

main: src/main.c build
	CC -W -I $(IDIR_SDL) -I $(IDIR_SDL_IMG) -L $(LDIR_SDL) -L $(LDIR_SDL_IMG) $(LLIB) src/cpu/cpu.c src/display/display.c src/main.c -o build/main

clean:
	rm -f build/main
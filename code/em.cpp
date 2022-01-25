#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef __EMSCRIPTEN__
#include <SDL/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <emscripten.h>
#endif

#include "controller.h"
#include "libretro.h"

struct EmButtons emButtons;
static SDL_Window* WindowOpenGL;
extern "C" bool retro_load_game_new(uint8_t* romdata,
                                    int size,
                                    bool loadEep,
                                    bool loadSra,
                                    bool loadFla);

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialize SDL\n");
    }

    WindowOpenGL =
        SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         640, 480, SDL_WINDOW_OPENGL);

    // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
#ifdef VBO
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#endif
    SDL_GLContext Context = SDL_GL_CreateContext(WindowOpenGL);

    printf("VENDOR: %s\n", glGetString(GL_VENDOR));
    printf("RENDERER: %s\n", glGetString(GL_RENDERER));
    printf("VERSION: %s\n", glGetString(GL_VERSION));
    printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    retro_init();

    // rom
    char rom_name[100];
    sprintf(rom_name, "%s", argv[1]);
    FILE* f = fopen(rom_name, "rb");
    fseek(f, 0, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* filecontent = (uint8_t*)malloc(fsize);
    fread(filecontent, 1, fsize, f);
    fclose(f);

    bool loaded = retro_load_game_new(filecontent, fsize, false /*loadEep*/,
                                      false /*loadSra*/, false /*loadFla*/);
    if (!loaded)
        printf("problem loading rom\n");

    return 0;
}

void mainLoop() {
    retro_run();
}

extern "C" void runMainLoop() {
    mainLoop();
}

extern "C" struct EmButtons* getEmButtons() {
    return &emButtons;
}

extern "C" void swapGl() {
    SDL_GL_SwapWindow(WindowOpenGL);
}

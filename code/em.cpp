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

static int UP      = 0x0001;
static int DOWN    = 0x0002;
static int LEFT    = 0x0004;
static int RIGHT   = 0x0008;
static int START   = 0x0010;
static int R_KEY   = 0x0020;
static int L_KEY   = 0x0040;
static int Z_KEY   = 0x0080;
static int A_KEY   = 0x0100;
static int B_KEY   = 0x0200;
static int CL_KEY  = 0x0400;
static int CR_KEY  = 0x0800;
static int CU_KEY  = 0x1000;
static int CD_KEY  = 0x2000;

extern "C" void updateControls(int buttons, int xAxis, int yAxis) {
  emButtons.upKey = buttons & UP ? true : false;
  emButtons.downKey = buttons & DOWN ? true : false;
  emButtons.leftKey = buttons & LEFT ? true : false;
  emButtons.rightKey = buttons & RIGHT ? true : false;
  emButtons.startKey = buttons & START ? true : false;
  emButtons.rKey = buttons & R_KEY ? true : false;
  emButtons.lKey = buttons & L_KEY ? true : false;
  emButtons.zKey = buttons & Z_KEY ? true : false;
  emButtons.aKey = buttons & A_KEY ? true : false;
  emButtons.bKey = buttons & B_KEY ? true : false;
  emButtons.axis0 = xAxis;
  emButtons.axis1 = yAxis;
  emButtons.cbLeft = buttons & CL_KEY ? true : false;
  emButtons.cbRight = buttons & CR_KEY ? true : false;
  emButtons.cbUp = buttons & CU_KEY ? true : false;
  emButtons.cbDown = buttons & CD_KEY ? true : false;
}
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

struct EmButtons emButtons[4];
static SDL_Window* WindowOpenGL;
extern "C" bool retro_load_game_new(uint8_t* romdata,
                                    int size,
                                    bool loadEep,
                                    bool loadSra,
                                    bool loadFla);

unsigned int fbo;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialize SDL\n");
    }

    // Disable keyboard capture
    SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
    SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
    SDL_EventState(SDL_KEYUP, SDL_DISABLE);

    // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifdef VBO
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#endif

    WindowOpenGL =
        SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext Context = SDL_GL_CreateContext(WindowOpenGL);

    glGenFramebuffers(1, &fbo);

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

int skip_frame = 0;
static int drew = 0;
static int current_wait = 0;
static int skip_count = 0;

void mainLoop() {
    if (skip_count > 0) {
        if (drew || current_wait >= skip_count) {
            skip_frame = !skip_frame;
            drew = 0;
            current_wait = 0;
        }
    }

    if (skip_frame) {
        current_wait++;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);  
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  
    }

    retro_run();
}

extern "C" void runMainLoop() {
    mainLoop();
}

extern "C" struct EmButtons* getEmButtons() {
    return emButtons;
}

extern "C" void swapGl() {
    drew = 1;
    if (!skip_frame) {
        SDL_GL_SwapWindow(WindowOpenGL);
    }
}

extern "C" void setSkipCount(int count) {
    skip_count = count;
}

extern "C" int getSkipCount() {
    return skip_count;
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

extern "C" void updateControls(int controller, int buttons, int xAxis, int yAxis) {
  emButtons[controller].upKey = buttons & UP ? true : false;
  emButtons[controller].downKey = buttons & DOWN ? true : false;
  emButtons[controller].leftKey = buttons & LEFT ? true : false;
  emButtons[controller].rightKey = buttons & RIGHT ? true : false;
  emButtons[controller].startKey = buttons & START ? true : false;
  emButtons[controller].rKey = buttons & R_KEY ? true : false;
  emButtons[controller].lKey = buttons & L_KEY ? true : false;
  emButtons[controller].zKey = buttons & Z_KEY ? true : false;
  emButtons[controller].aKey = buttons & A_KEY ? true : false;
  emButtons[controller].bKey = buttons & B_KEY ? true : false;
  emButtons[controller].axis0 = xAxis;
  emButtons[controller].axis1 = yAxis;
  emButtons[controller].cbLeft = buttons & CL_KEY ? true : false;
  emButtons[controller].cbRight = buttons & CR_KEY ? true : false;
  emButtons[controller].cbUp = buttons & CU_KEY ? true : false;
  emButtons[controller].cbDown = buttons & CD_KEY ? true : false;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libretro.h>

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <GL/glew.h>
#include <glsm/glsm.h>

#endif

#include "../glide2gl/src/Glitch64/glide.h"
#include "api/m64p_frontend.h"
#include "api/m64p_types.h"
#include "dd/dd_disk.h"
#include "libretro_memory.h"
#include "main/cheat.h"
#include "main/main.h"
#include "main/savestates.h"
#include "main/version.h"
#include "memory/memory.h"
#include "pi/pi_controller.h"
#include "plugin/plugin.h"
#include "r4300/r4300.h"
#include "si/pif.h"

/* Cxd4 RSP */
#include "../Graphics/plugin.h"
#include "../mupen64plus-rsp-cxd4/config.h"
#include "plugin/audio_libretro/audio_plugin.h"

#ifndef PRESCALE_WIDTH
#define PRESCALE_WIDTH 640
#endif

#ifndef PRESCALE_HEIGHT
#define PRESCALE_HEIGHT 625
#endif

/* forward declarations */
int InitGfx(void);
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
int glide64InitGfx(void);
void gles2n64_reset(void);
#endif

struct retro_rumble_interface rumble;

#define SUBSYSTEM_CART_DISK 0x0101

static const struct retro_subsystem_rom_info n64_cart_disk[] = {
    {"Cartridge", "n64|z64|v64|bin", false, false, false, NULL, 0},
    {"Disk", "ndd|bin", false, false, false, NULL, 0},
    {NULL}};

static const struct retro_subsystem_info subsystems[] = {
    {"Cartridge and Disk", "n64_cart_disk", n64_cart_disk, 2,
     SUBSYSTEM_CART_DISK},
    {NULL}};

save_memory_data saved_memory;

bool stop_stepping;

float polygonOffsetFactor = 0.0f;
float polygonOffsetUnits = 0.0f;

static bool vulkan_inited = false;
static bool gl_inited = false;

int astick_deadzone = 1000;
int astick_sensitivity = 100;
int first_time = 1;
bool flip_only = false;

static uint8_t* cart_data = NULL;
static uint32_t cart_size = 0;
static uint8_t* disk_data = NULL;
static uint32_t disk_size = 0;

static bool emu_initialized = false;
static unsigned audio_buffer_size = 2048;

static unsigned retro_filtering = 0;
static unsigned retro_dithering = 0;
static bool reinit_screen = false;
static bool first_context_reset = false;
static bool pushed_frame = false;

bool frame_dupe = false;

uint32_t gfx_plugin_accuracy = 2;
static enum rsp_plugin_type rsp_plugin;
uint32_t screen_width = 640;
uint32_t screen_height = 480;
uint32_t screen_pitch = 0;
uint32_t screen_aspectmodehint;
uint32_t send_allist_to_hle_rsp = 0;

unsigned int BUFFERSWAP = 0;
unsigned int FAKE_SDL_TICKS = 0;

bool alternate_mapping;

extern int g_vi_refresh_rate;

/* after the controller's CONTROL* member has been assigned we can update
 * them straight from here... */
extern struct {
    CONTROL* control;
    BUTTONS buttons;
} controller[4];

/* ...but it won't be at least the first time we're called, in that case set
 * these instead for input_plugin to read. */
int pad_pak_types[4];
int pad_present[4] = {1, 0, 0, 0};

void log_cb(int type, char* message) {
    printf("%s", message);
}

static void n64DebugCallback(void* aContext, int aLevel, const char* aMessage) {
    char buffer[1024];

    sprintf(buffer, "mupen64plus: %s\n", aMessage);

    switch (aLevel) {
        case M64MSG_ERROR:
            log_cb(RETRO_LOG_ERROR, buffer);
            break;
        case M64MSG_INFO:
            log_cb(RETRO_LOG_INFO, buffer);
            break;
        case M64MSG_WARNING:
            log_cb(RETRO_LOG_WARN, buffer);
            break;
        case M64MSG_VERBOSE:
        case M64MSG_STATUS:
            log_cb(RETRO_LOG_DEBUG, buffer);
            break;
        default:
            break;
    }
}

extern m64p_rom_header ROM_HEADER;

static void core_settings_autoselect_gfx_plugin(void) {
    gfx_plugin = GFX_GLIDE64;
}

static void core_settings_autoselect_rsp_plugin(void) {
    rsp_plugin = RSP_HLE;
}

unsigned libretro_get_gfx_plugin(void) {
    return gfx_plugin;
}

static void core_settings_set_defaults(void) {
    core_settings_autoselect_gfx_plugin();
    rsp_plugin = RSP_HLE;
}

bool is_cartridge_rom(const uint8_t* data) {
    return (data != NULL && *((uint32_t*)data) != 0x16D348E8 &&
            *((uint32_t*)data) != 0x56EE6322);
}

static bool emu_step_load_data() {
    const char* dir = "empty";
    bool loaded = false;
    char slash;

#if defined(_WIN32)
    slash = '\\';
#else
    slash = '/';
#endif

    if (CoreStartup(FRONTEND_API_VERSION, ".", ".", "Core", n64DebugCallback, 0,
                    0) &&
        log_cb)
        log_cb(RETRO_LOG_ERROR, "mupen64plus: Failed to initialize core\n");

    if (cart_data != NULL && cart_size != 0) {
        /* N64 Cartridge loading */
        loaded = true;

        if (log_cb)
            log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_ROM_OPEN\n");

        if (CoreDoCommand(M64CMD_ROM_OPEN, cart_size, (void*)cart_data)) {
            if (log_cb)
                log_cb(RETRO_LOG_ERROR, "mupen64plus: Failed to load ROM\n");
            goto load_fail;
        }

        free(cart_data);
        cart_data = NULL;

        if (log_cb)
            log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_ROM_GET_HEADER\n");

        if (CoreDoCommand(M64CMD_ROM_GET_HEADER, sizeof(ROM_HEADER),
                          &ROM_HEADER)) {
            if (log_cb)
                log_cb(RETRO_LOG_ERROR,
                       "mupen64plus; Failed to query ROM header information\n");
            goto load_fail;
        }
    }
    if (disk_data != NULL && disk_size != 0) {
        /* 64DD Disk loading */
        char disk_ipl_path[256];
        FILE* fPtr;
        long romlength = 0;
        uint8_t* ipl_data = NULL;

        loaded = true;
        // TODO: Support for DD?
        // if (!environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) ||
        // !dir)
        //     goto load_fail;

        /* connect saved_memory.disk to disk */
        g_dd_disk = saved_memory.disk;

        if (log_cb)
            log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_DISK_OPEN\n");
        printf("M64CMD_DISK_OPEN\n");

        if (CoreDoCommand(M64CMD_DISK_OPEN, disk_size, (void*)disk_data)) {
            if (log_cb)
                log_cb(RETRO_LOG_ERROR, "mupen64plus: Failed to load DISK\n");
            goto load_fail;
        }

        free(disk_data);
        disk_data = NULL;

        /* 64DD IPL LOAD - assumes "64DD_IPL.bin" is in system folder */
        sprintf(disk_ipl_path, "%s%c64DD_IPL.bin", dir, slash);

        fPtr = fopen(disk_ipl_path, "rb");
        if (fPtr == NULL) {
            if (log_cb)
                log_cb(RETRO_LOG_ERROR,
                       "mupen64plus: Failed to load DISK IPL\n");
            goto load_fail;
        }

        fseek(fPtr, 0L, SEEK_END);
        romlength = ftell(fPtr);
        fseek(fPtr, 0L, SEEK_SET);

        ipl_data = malloc(romlength);
        if (ipl_data == NULL) {
            if (log_cb)
                log_cb(RETRO_LOG_ERROR,
                       "mupen64plus: couldn't allocate DISK IPL buffer\n");
            fclose(fPtr);
            free(ipl_data);
            ipl_data = NULL;
            goto load_fail;
        }

        if (fread(ipl_data, 1, romlength, fPtr) != romlength) {
            if (log_cb)
                log_cb(RETRO_LOG_ERROR,
                       "mupen64plus: couldn't read DISK IPL file to buffer\n");
            fclose(fPtr);
            free(ipl_data);
            ipl_data = NULL;
            goto load_fail;
        }
        fclose(fPtr);

        if (log_cb)
            log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_DDROM_OPEN\n");
        printf("M64CMD_DDROM_OPEN\n");

        if (CoreDoCommand(M64CMD_DDROM_OPEN, romlength, (void*)ipl_data)) {
            if (log_cb)
                log_cb(RETRO_LOG_ERROR, "mupen64plus: Failed to load DDROM\n");
            free(ipl_data);
            ipl_data = NULL;
            goto load_fail;
        }

        if (log_cb)
            log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_ROM_GET_HEADER\n");

        if (CoreDoCommand(M64CMD_ROM_GET_HEADER, sizeof(ROM_HEADER),
                          &ROM_HEADER)) {
            if (log_cb)
                log_cb(RETRO_LOG_ERROR,
                       "mupen64plus; Failed to query ROM header information\n");
            goto load_fail;
        }
    }
    return loaded;

load_fail:
    free(cart_data);
    cart_data = NULL;
    free(disk_data);
    disk_data = NULL;
    stop = 1;

    return false;
}

bool emu_step_render(void);

int retro_return(bool just_flipping) {
    if (stop) return 0;

    vbo_disable();

    if (just_flipping) {
        /* HACK: in case the VI comes before the render? is that possible?
         * remove this when we totally remove libco */
        flip_only = 1;
        emu_step_render();
        flip_only = 0;
    } else
        flip_only = just_flipping;

    stop_stepping = true;

    return 0;
}

int retro_stop_stepping(void) {
    return stop_stepping;
}

extern void swapGl();

bool emu_step_render(void) {
    if (flip_only) {
        // video_cb(RETRO_HW_FRAME_BUFFER_VALID, screen_width, screen_height,
        // 0);
        swapGl();
        pushed_frame = true;
        return true;
    }

    // if (!pushed_frame && frame_dupe) /* Dupe. Not duping violates libretro
    // API, consider it a speedhack. */
    //    video_cb(NULL, screen_width, screen_height, screen_pitch);
    return false;
}

static void emu_step_initialize(void) {
    if (emu_initialized)
        return;

    emu_initialized = true;

    core_settings_set_defaults();
    core_settings_autoselect_gfx_plugin();
    core_settings_autoselect_rsp_plugin();

    plugin_connect_all(gfx_plugin, rsp_plugin);

    if (log_cb)
        log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_EXECUTE.\n");

    CoreDoCommand(M64CMD_EXECUTE, 0, NULL);
}

void reinit_gfx_plugin(void) {
    if (first_context_reset) {
        first_context_reset = false;
    }

    glide64InitGfx();
}

void deinit_gfx_plugin(void) {}

static void EmuThreadInit(void) {
    emu_step_initialize();
    main_pre_run();
}

static void EmuThreadStep(void) {
    stop_stepping = false;
    main_run();
}

/* Get the system type associated to a ROM country code. */
static m64p_system_type rom_country_code_to_system_type(char country_code) {
    switch (country_code) {
            /* PAL codes */
        case 0x44:
        case 0x46:
        case 0x49:
        case 0x50:
        case 0x53:
        case 0x55:
        case 0x58:
        case 0x59:
            return SYSTEM_PAL;

            /* NTSC codes */
        case 0x37:
        case 0x41:
        case 0x45:
        case 0x4a:
        default: /* Fallback for unknown codes */
            return SYSTEM_NTSC;
    }
}

int isPalSystem() {
    struct retro_system_av_info info;
    retro_get_system_av_info(&info);
    m64p_system_type region =
        rom_country_code_to_system_type(ROM_HEADER.destination_code);
    return region == SYSTEM_PAL;
}

void retro_get_system_av_info(struct retro_system_av_info* info) {
    m64p_system_type region =
        rom_country_code_to_system_type(ROM_HEADER.destination_code);

    info->geometry.base_width = screen_width;
    info->geometry.base_height = screen_height;
    info->geometry.max_width = screen_width;
    info->geometry.max_height = screen_height;
    info->geometry.aspect_ratio = 4.0 / 3.0;
    info->timing.fps =
        (region == SYSTEM_PAL) ? 50.0 : (60.13); /* TODO: Actual timing  */
    info->timing.sample_rate = 44100.0;
}

static void context_reset(void) {
    static bool first_init = true;
    printf("context_reset.\n");
    glsm_ctl(GLSM_CTL_STATE_CONTEXT_RESET, NULL);
    if (first_init) {
        glsm_ctl(GLSM_CTL_STATE_SETUP, NULL);
        first_init = false;
    }
    reinit_gfx_plugin();
}

static void context_destroy(void) {
    deinit_gfx_plugin();
}

static bool context_framebuffer_lock(void* data) {
    if (!stop)
        return false;
    return true;
}

static bool retro_init_gl(void) {
    glsm_ctx_params_t params = {0};

    params.context_reset = context_reset;
    params.context_destroy = context_destroy;
    params.stencil = false;
    params.framebuffer_lock = context_framebuffer_lock;

    if (!glsm_ctl(GLSM_CTL_STATE_CONTEXT_INIT, &params)) {
        if (log_cb)
            log_cb(RETRO_LOG_ERROR,
                   "mupen64plus: libretro frontend doesn't have OpenGL "
                   "support.\n");
        return false;
    }

    context_reset();

    return true;
}

void retro_init(void) {
    screen_pitch = 0;
    /* hacky stuff for Glide64 */
    polygonOffsetUnits = -3.0f;
    polygonOffsetFactor = -3.0f;
}

void retro_deinit(void) {
    mupen_main_stop();
    mupen_main_exit();

    deinit_audio_libretro();

    gl_inited = false;
}

extern void glide_set_filtering(unsigned value);
extern void ChangeSize();

static void gfx_set_filtering(void) {
    if (log_cb)
        log_cb(RETRO_LOG_DEBUG, "set filtering mode...\n");
    glide_set_filtering(retro_filtering);
}

unsigned setting_get_dithering(void) {
    return retro_dithering;
}

void update_variables(bool startup) {

    send_allist_to_hle_rsp = false;
    screen_width = 640;
    screen_height = 480;

    if (startup) {
        core_settings_autoselect_gfx_plugin();
    }

    CFG_HLE_GFX = 0;
    CFG_HLE_AUD = 0; /* There is no HLE audio code in libretro audio plugin. */

#ifdef DISABLE_3POINT
    retro_filtering = 3;
#else
    retro_filtering = 1;
#endif

    gfx_set_filtering();
    retro_dithering = 1;
    gfx_plugin_accuracy = 2;
    BUFFERSWAP = false;
    frame_dupe = false;
    alternate_mapping = false;
    int p1_pak = PLUGIN_MEMPAK;
    // p1_pak = PLUGIN_RAW;
    // p1_pak = PLUGIN_MEMPAK;

    if (controller[0].control)
        controller[0].control->Plugin = p1_pak;
    else
        pad_pak_types[0] = p1_pak;
    if (controller[1].control)
        controller[1].control->Plugin = p1_pak;
    else
        pad_pak_types[1] = p1_pak;
    if (controller[2].control)
        controller[2].control->Plugin = p1_pak;
    else
        pad_pak_types[2] = p1_pak;
    if (controller[3].control)
        controller[3].control->Plugin = p1_pak;
    else
        pad_pak_types[3] = p1_pak;
}

char* loadFile(char* filename, int* size) {
    FILE* f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0, SEEK_SET); /* same as rewind(f); */

    char* filecontent = (char*)malloc(fsize);
    int readsize = fread(filecontent, 1, fsize, f);
    fclose(f);

    *size = readsize;

    // filecontent[readsize] = '\0';

    return filecontent;
}

static void format_saved_memory(bool loadEep, bool loadSra, bool loadFla) {
    format_sram(saved_memory.sram);
    format_eeprom(saved_memory.eeprom, sizeof(saved_memory.eeprom));
    format_flashram(saved_memory.flashram);

    if (loadEep) {
        int size = 0;
        char* eep = loadFile("game.eep", &size);
        memcpy(saved_memory.eeprom, eep, size);
        printf("eep loaded\n");
    }
    if (loadSra) {
        int size = 0;
        char* sra = loadFile("game.sra", &size);
        memcpy(saved_memory.sram, sra, size);
        printf("sra loaded\n");
    }
    if (loadFla) {
        int size = 0;
        char* fla = loadFile("game.fla", &size);
        memcpy(saved_memory.flashram, fla, size);
        printf("fla loaded\n");
    }

    format_mempak(saved_memory.mempack[0]);
    format_mempak(saved_memory.mempack[1]);
    format_mempak(saved_memory.mempack[2]);
    format_mempak(saved_memory.mempack[3]);
    format_disk(saved_memory.disk);
}

bool retro_load_game_new(uint8_t* romdata,
                         int size,
                         bool loadEep,
                         bool loadSra,
                         bool loadFla) {

    format_saved_memory(loadEep, loadSra, loadFla);
    update_variables(true);
    init_audio_libretro(audio_buffer_size);

    {
        retro_init_gl();
        gl_inited = true;
    }

    if (gl_inited) {
        gfx_plugin = GFX_GLIDE64;
        rsp_plugin = RSP_HLE;
    }

    if (is_cartridge_rom(romdata)) {
        cart_data = malloc(size);
        cart_size = size;
        memcpy(cart_data, romdata, size);
    } else {
        disk_data = malloc(size);
        disk_size = size;
        memcpy(disk_data, romdata, size);
    }

    stop = false;
    /* Finish ROM load before doing anything funny,
     * so we can return failure if needed. */
    emu_step_load_data();

    if (stop) return false;

    first_context_reset = true;

    return true;
}

void retro_run(void) {
    FAKE_SDL_TICKS += 16;
    pushed_frame = false;

    {
        if (stop) return;
        glsm_ctl(GLSM_CTL_STATE_BIND, NULL);

        if (first_time) {
            first_time = 0;
            emu_step_initialize();
            /* Additional check for vi overlay not set at start */
            update_variables(false);
            gfx_set_filtering();
            EmuThreadInit();
        }

        EmuThreadStep();

        if (stop) return;
        glsm_ctl(GLSM_CTL_STATE_UNBIND, NULL);
    }
}

MAKEFLAGS += --no-builtin-rules
MAKE = /usr/bin/make
CFILES := src/glide2gl/src/Glide64/Combine.c \
    src/glide2gl/src/Glide64/Framebuffer_glide64.c \
    src/glide2gl/src/Glide64/glide64_3dmath.c \
    src/glide2gl/src/Glide64/glide64_gDP.c \
    src/glide2gl/src/Glide64/glide64_gSP.c \
    src/glide2gl/src/Glide64/Glide64_Ini.c \
    src/glide2gl/src/Glide64/glide64_rdp.c \
    src/glide2gl/src/Glide64/Glide64_UCode.c \
    src/glide2gl/src/Glide64/glide64_util.c \
    src/glide2gl/src/Glide64/glidemain.c \
    src/glide2gl/src/Glide64/MiClWr.c \
    src/glide2gl/src/Glide64/TexCache.c \
    src/glide2gl/src/Glide64/TexLoad.c \
    src/glide2gl/src/Glitch64/geometry.c \
    src/glide2gl/src/Glitch64/glitch64_combiner.c \
    src/glide2gl/src/Glitch64/glitch64_textures.c \
    src/glide2gl/src/Glitch64/glitchmain.c \
    src/Graphics/3dmaths.c \
    src/Graphics/HLE/Microcode/Fast3D.c \
    src/Graphics/plugins.c \
    src/Graphics/RDP/gDP_funcs_C.c \
    src/Graphics/RDP/gDP_state.c \
    src/Graphics/RDP/RDP_state.c \
    src/Graphics/RSP/gSP_funcs_C.c \
    src/Graphics/RSP/gSP_state.c \
    src/Graphics/RSP/RSP_state.c \
    src/libretro-common/audio/conversion/float_to_s16.c \
    src/libretro-common/audio/conversion/s16_to_float.c \
    src/libretro-common/audio/resampler/audio_resampler.c \
    src/libretro-common/audio/resampler/drivers/nearest_resampler.c \
    src/libretro-common/audio/resampler/drivers/null_resampler.c \
    src/libretro-common/audio/resampler/drivers/sinc_resampler.c \
    src/libretro-common/compat/compat_posix_string.c \
    src/libretro-common/compat/compat_snprintf.c \
    src/libretro-common/compat/compat_strcasestr.c \
    src/libretro-common/compat/compat_strl.c \
    src/libretro-common/compat/fopen_utf8.c \
    src/libretro-common/encodings/encoding_crc32.c \
    src/libretro-common/encodings/encoding_utf.c \
    src/libretro-common/features/features_cpu.c \
    src/libretro-common/file/config_file.c \
    src/libretro-common/file/config_file_userdata.c \
    src/libretro-common/file/file_path.c \
    src/libretro-common/gfx/gl_capabilities.c \
    src/libretro-common/glsm/glsm.c \
    src/libretro-common/glsym/rglgen.c \
    src/libretro-common/hash/rhash.c \
    src/libretro-common/lists/string_list.c \
    src/libretro-common/memmap/memalign.c \
    src/libretro-common/streams/file_stream.c \
    src/libretro-common/string/stdstring.c \
    src/libretro-common/vfs/vfs_implementation.c \
    src/libretro/brumme_crc.c \
    src/libretro/libretro_em.c \
    src/mupen64plus-core/src/ai/ai_controller.c \
    src/mupen64plus-core/src/api/callbacks.c \
    src/mupen64plus-core/src/api/common.c \
    src/mupen64plus-core/src/api/config.c \
    src/mupen64plus-core/src/api/frontend.c \
    src/mupen64plus-core/src/api/vidext_libretro.c \
    src/mupen64plus-core/src/dd/dd_controller.c \
    src/mupen64plus-core/src/dd/dd_disk.c \
    src/mupen64plus-core/src/dd/dd_rom.c \
    src/mupen64plus-core/src/gb/gb_cart.c \
    src/mupen64plus-core/src/main/cheat.c \
    src/mupen64plus-core/src/main/device.c \
    src/mupen64plus-core/src/main/eventloop.c \
    src/mupen64plus-core/src/main/main.c \
    src/mupen64plus-core/src/main/md5.c \
    src/mupen64plus-core/src/main/profile.c \
    src/mupen64plus-core/src/main/rom.c \
    src/mupen64plus-core/src/main/savestates.c \
    src/mupen64plus-core/src/main/util.c \
    src/mupen64plus-core/src/memory/m64p_memory.c \
    src/mupen64plus-core/src/pifbootrom/pifbootrom.c \
    src/mupen64plus-core/src/pi/cart_rom.c \
    src/mupen64plus-core/src/pi/flashram.c \
    src/mupen64plus-core/src/pi/pi_controller.c \
    src/mupen64plus-core/src/pi/sram.c \
    src/mupen64plus-core/src/plugin/audio_libretro/audio_backend_libretro.c \
    src/mupen64plus-core/src/plugin/emulate_game_controller_via_libretro.c \
    src/mupen64plus-core/src/plugin/get_time_using_C_localtime.c \
    src/mupen64plus-core/src/plugin/plugin.c \
    src/mupen64plus-core/src/plugin/rumble_via_input_plugin.c \
    src/mupen64plus-core/src/r4300/cached_interp.c \
    src/mupen64plus-core/src/r4300/cp0.c \
    src/mupen64plus-core/src/r4300/cp1.c \
    src/mupen64plus-core/src/r4300/empty_dynarec.c \
    src/mupen64plus-core/src/r4300/exception.c \
    src/mupen64plus-core/src/r4300/instr_counters.c \
    src/mupen64plus-core/src/r4300/interrupt.c \
    src/mupen64plus-core/src/r4300/mi_controller.c \
    src/mupen64plus-core/src/r4300/pure_interp.c \
    src/mupen64plus-core/src/r4300/r4300.c \
    src/mupen64plus-core/src/r4300/r4300_core.c \
    src/mupen64plus-core/src/r4300/recomp.c \
    src/mupen64plus-core/src/r4300/reset.c \
    src/mupen64plus-core/src/r4300/tlb.c \
    src/mupen64plus-core/src/rdp/fb.c \
    src/mupen64plus-core/src/rdp/rdp_core.c \
    src/mupen64plus-core/src/ri/rdram.c \
    src/mupen64plus-core/src/ri/rdram_detection_hack.c \
    src/mupen64plus-core/src/ri/ri_controller.c \
    src/mupen64plus-core/src/rsp/rsp_core.c \
    src/mupen64plus-core/src/si/af_rtc.c \
    src/mupen64plus-core/src/si/cic.c \
    src/mupen64plus-core/src/si/eeprom.c \
    src/mupen64plus-core/src/si/game_controller.c \
    src/mupen64plus-core/src/si/mempak.c \
    src/mupen64plus-core/src/si/n64_cic_nus_6105.c \
    src/mupen64plus-core/src/si/pif.c \
    src/mupen64plus-core/src/si/rumblepak.c \
    src/mupen64plus-core/src/si/si_controller.c \
    src/mupen64plus-core/src/si/transferpak.c \
    src/mupen64plus-core/src/vi/vi_controller.c \
    src/mupen64plus-rsp-cxd4/rsp.c \
    src/mupen64plus-rsp-hle/src/alist.c \
    src/mupen64plus-rsp-hle/src/alist_audio.c \
    src/mupen64plus-rsp-hle/src/alist_naudio.c \
    src/mupen64plus-rsp-hle/src/alist_nead.c \
    src/mupen64plus-rsp-hle/src/audio.c \
    src/mupen64plus-rsp-hle/src/cicx105.c \
    src/mupen64plus-rsp-hle/src/hle.c \
    src/mupen64plus-rsp-hle/src/hle_memory.c \
    src/mupen64plus-rsp-hle/src/hle_plugin.c \
    src/mupen64plus-rsp-hle/src/jpeg.c \
    src/mupen64plus-rsp-hle/src/mp3.c \
    src/mupen64plus-rsp-hle/src/musyx.c \
    src/mupen64plus-rsp-hle/src/re2.c

CPPFILES := em.cpp \
	src/Graphics/RDP/gDP_funcs.cpp \
	src/Graphics/RSP/gSP_funcs.cpp

FILES := $(patsubst %.c,%.o,$(CFILES)) $(patsubst %.cpp,%.o,$(CPPFILES))

FLAGS := \
    $(CFLAGS) \
	-I. \
	-I./src/include \
    -I./src/glide2gl/src/Glide64 \
    -I./src/libretro \
	-I./src/libretro-common/include \
    -I./src/mupen64plus-core/src \
	-I./src/mupen64plus-core/src/api \
	-I./src/mupen64plus-core/src/plugin/audio_libretro \
    -DWRC \
	-DHAVE_OPENGLES \
	-DHAVE_OPENGLES2 \
	-DHAVE_GLIDE64 \
	-DNDEBUG \
	-D__LIBRETRO__ \
	-DM64P_PLUGIN_API \
	-DM64P_CORE_PROTOTYPES \
	-D_ENDUSER_RELEASE \
	-DSINC_LOWER_QUALITY \
	-DNOSSE \
	-DNO_ASM \
	-DNO_LIBCO \
	-DDISABLE_3POINT \
	-DINLINE="inline" \
	-O3 \
	-s USE_SDL=2 \
    -Wno-c++11-narrowing \
    -fno-rtti \
    -fno-exceptions \
    -flto

#536870912
#268435456
#201326592
#144179200

LINK_FLAGS := \
    -O3 \
    -lGL \
    -s USE_SDL=2 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="'n64'" \
    -s USE_ZLIB=1 \
    -s TOTAL_MEMORY=144179200 \
    -s ALLOW_MEMORY_GROWTH=1 \
	-s ASSERTIONS=0 \
	-s EXIT_RUNTIME=0 \
	-s EXPORTED_RUNTIME_METHODS="['callMain','cwrap', 'FS']" \
    -s EXPORTED_FUNCTIONS="['_main','_runMainLoop', '_isPalSystem', '_isVboEnabled', '_updateControls']" \
	-s INVOKE_RUN=0 \
    -flto

all: n64wasm n64wasm-vbo

n64wasm:
	@echo n64wasm
	$(MAKE) clean n64wasm.js

n64wasm-vbo:
	@echo n64wasm-vbo
	$(MAKE) CFLAGS=-DVBO clean n64wasm-vbo.js 

n64wasm.js: $(FILES)
	emcc -o $@ $(FILES) $(LINK_FLAGS) -s FULL_ES3=1

n64wasm-vbo.js: $(FILES)
	emcc -o $@ $(FILES) $(LINK_FLAGS) -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2

%.o : %.cpp
	emcc -c $< -o $@ \
	$(FLAGS)

%.o : %.c
	emcc -c $< -o $@ \
	$(FLAGS)

clean:
	@echo "Cleaning"
	@echo $(FILES)
	rm -fr *.o */*.o */*/*.o */*/*/*.o */*/*/*/*.o	

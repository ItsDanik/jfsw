# CMake build of the game for the MiSTer hybrid core (and of the same code
# for a PC, to test it). Every other platform uses the Makefiles.
#
# Classic software renderer only: no Polymost, no OpenGL, no editor.
cmake_minimum_required(VERSION 3.16)
project(jfsw C)

option(MISTER_HYBRID "Build for the MiSTer hybrid core (needs SDL2 with the MiSTer drivers)" OFF)
if(NOT MISTER_HYBRID)
	message(FATAL_ERROR "This CMake build is for the MiSTer hybrid core (-DMISTER_HYBRID=ON). Use the Makefile otherwise.")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)

find_package(SDL2 REQUIRED CONFIG)
# mister_hybrid.h is installed next to the SDL headers
get_target_property(SDL2_INCLUDES SDL2::SDL2-static INTERFACE_INCLUDE_DIRECTORIES)

# Music: the tracks of the CD as Ogg Vorbis files
find_library(VORBISFILE_LIBRARY vorbisfile REQUIRED)
find_library(VORBIS_LIBRARY vorbis REQUIRED)
find_library(OGG_LIBRARY ogg REQUIRED)
find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h REQUIRED)

set(ENGINE jfbuild/src)
set(MACT jfmact)
set(AUDIO jfaudiolib/src)

add_executable(jfsw
	${ENGINE}/a-c.c
	${ENGINE}/asmprot.c
	${ENGINE}/baselayer.c
	${ENGINE}/cache1d.c
	${ENGINE}/compat.c
	${ENGINE}/crc32.c
	${ENGINE}/defs.c
	${ENGINE}/engine.c
	${ENGINE}/kplib.c
	${ENGINE}/mmulti.c
	${ENGINE}/osd.c
	${ENGINE}/pragmas.c
	${ENGINE}/scriptfile.c
	${ENGINE}/textfont.c
	${ENGINE}/talltextfont.c
	${ENGINE}/smalltextfont.c
	${ENGINE}/sdlayer2.c
	${ENGINE}/startwin_stub.c
	${ENGINE}/version.c

	${MACT}/util_lib.c
	${MACT}/file_lib.c
	${MACT}/control.c
	${MACT}/keyboard.c
	${MACT}/mouse.c
	${MACT}/mathutil.c
	${MACT}/scriplib.c
	${MACT}/animlib.c

	${AUDIO}/drivers.c
	${AUDIO}/fx_man.c
	${AUDIO}/cd.c
	${AUDIO}/multivoc.c
	${AUDIO}/mix.c
	${AUDIO}/mixst.c
	${AUDIO}/pitch.c
	${AUDIO}/vorbis.c
	${AUDIO}/music.c
	${AUDIO}/midi.c
	${AUDIO}/driver_nosound.c
	${AUDIO}/driver_sdl.c
	${AUDIO}/asssys.c

	src/actor.c
	src/ai.c
	src/anim.c
	src/border.c
	src/break.c
	src/bunny.c
	src/cache.c
	src/cheats.c
	src/colormap.c
	src/config.c
	src/console.c
	src/coolg.c
	src/coolie.c
	src/copysect.c
	src/demo.c
	src/draw.c
	src/eel.c
	src/game.c
	src/girlninj.c
	src/goro.c
	src/grpscan.c
	src/hornet.c
	src/interp.c
	src/interpsh.c
	src/inv.c
	src/jplayer.c
	src/jsector.c
	src/jweapon.c
	src/lava.c
	src/light.c
	src/mclip.c
	src/menus.c
	src/miscactr.c
	src/morph.c
	src/net.c
	src/ninja.c
	src/osdcmds.c
	src/osdfuncs.c
	src/panel.c
	src/player.c
	src/predict.c
	src/quake.c
	src/ripper.c
	src/ripper2.c
	src/rooms.c
	src/rotator.c
	src/rts.c
	src/save.c
	src/scrip2.c
	src/sector.c
	src/serp.c
	src/setup.c
	src/skel.c
	src/skull.c
	src/slidor.c
	src/sounds.c
	src/spike.c
	src/sprite.c
	src/sumo.c
	src/swconfig.c
	src/sync.c
	src/text.c
	src/track.c
	src/vator.c
	src/vis.c
	src/wallmove.c
	src/warp.c
	src/weapon.c
	src/zilla.c
	src/zombie.c
	src/saveable.c
	src/version.c
	src/mister_port.c
)

target_include_directories(jfsw PRIVATE
	src jfbuild/include jfbuild/src jfmact jfaudiolib/include jfaudiolib/src
	${SDL2_INCLUDES} ${VORBIS_INCLUDE_DIR})
target_compile_definitions(jfsw PRIVATE
	MISTER_HYBRID=1 RENDERTYPESDL=1 HAVE_SDL=2 HAVE_VORBIS _GNU_SOURCE)
target_compile_options(jfsw PRIVATE -fno-strict-aliasing -w)
target_link_libraries(jfsw PRIVATE
	${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY} SDL2::SDL2-static m)

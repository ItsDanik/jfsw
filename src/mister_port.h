#ifndef MISTER_PORT_H
#define MISTER_PORT_H

// MiSTer hybrid core: the game runs on the ARM, the FPGA core shows the
// picture at 15kHz and provides audio and input. SDL's "mister" drivers
// (hybrid/sdl2) do most of the work; this is what is specific to the game.
//
// Everything is a no-op without the core. The engine's part is in
// jfbuild/src/sdlayer2.c: video, the clock and the joystick.

#ifdef __cplusplus
extern "C" {
#endif

// In app_main(), before the engine is started
void MiSTer_Init(void);
// The folders of a copy of "Shadow Warrior Classic Redux" (Steam, GOG) that
// have the addons and the music, so that the whole game folder can be copied
void MiSTer_AddSearchPaths(void);
// "Loading..." on the screen, until the game shows its first frame
void MiSTer_Loading(void);

struct grpfile;
// After ScanGroups(): with more than one game in the folder the player picks
// one on the screen. Returns the game to start, `selected` if there is nothing
// to pick; quits if the player leaves the list.
struct grpfile const *MiSTer_SelectGroup(struct grpfile const *selected);
// No GRP file: tells the player which files to copy where
void MiSTer_NoGameData(void);

// Before the game starts: lets the player pick one of several data sets on
// the screen. Returns the index, -1 to quit
int MiSTer_PickGame(const char *const *names, int count, int selected);
// Problems the player can fix (missing data): shown on the screen until a
// button is pressed, in plain words that say which files go where
void MiSTer_ShowError(const char *message);
// Exit code of the process after the player quit: the launcher starts the
// game again (back to the list of MiSTer_PickGame) or returns to the menu
int MiSTer_ExitCode(void);

#ifdef __cplusplus
}
#endif

#endif

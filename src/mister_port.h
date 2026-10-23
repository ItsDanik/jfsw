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

// OSD options of the game (CONF_STR in core/jfSW.sv of jfSW_MiSTer). The first
// entry of each is 0, the default.
// Resolution: 320 or 640 wide, 200 or 240 lines
int MiSTer_ScreenWidth(void);
int MiSTer_ScreenHeight(void);
// Left (0) and right (1) stick sensitivity, in percent of the whole axis
int MiSTer_StickSensitivity(int stick);
// Limit Vertical Look: narrows the range of the horizon around its centre to
// a half. Leaves the game's own limits when it is off
void MiSTer_HorizLimits(int centre, int *min, int *max);
// Once per frame, between two frames: follows the OSD options that apply
// while the game runs (draw.c)
void MiSTer_UpdateOptions(void);
// 240 lines (hybrid/CLAUDE.md, "240 lines and HUD margins"): the lines
// between the status bar (or the mini bar) and the bottom edge, the OSD's
// HUD (Bottom); 0 on a screen of 200 lines. The HUD line of the OSD is
// greyed out there (`lines` is the screen's height).
int MiSTer_HudBottom(int lines);

// Cheats page of the OSD: God Mode and No Clipping on (1) or off (0), and
// the triggers (MH_OSDTriggers(), T[56] and T[57]) selected since the last
// call. MiSTer_Cheats() (cheats.c) applies them once a frame while a level is
// played.
int MiSTer_CheatGodMode(void);
int MiSTer_CheatNoClip(void);
enum { MISTER_CHEAT_GIVEALL = 1, MISTER_CHEAT_NEXTLEVEL = 2 };
int MiSTer_CheatTriggers(void);
void MiSTer_Cheats(void);

// In app_main(), before the engine is started
void MiSTer_Init(void);
// The folders of a copy of "Shadow Warrior Classic Redux" (Steam, GOG) that
// have the addons and the music, so that the whole game folder can be copied
void MiSTer_AddSearchPaths(void);
// The core opened before the games are looked for; nothing on its screen of
// our own: the data check's lines and "Loading" come with the pick
// (MH_PickGame(), host version 17, the same in every core)
void MiSTer_Loading(void);
// A file's name as it is in the game's folder or its search paths, in
// whatever case (the CD tracks: Track02.ogg for the game's track02.ogg)
void MiSTer_FileCase(char *name, int size);

struct grpfile;
// After ScanGroups(): the game to start (MH_PickGame()), the one of the
// core's menu entry, or the player's pick from a list of those that are there
// when the core was started by itself. Quits after a message when it is not
// there, or when the player leaves the list.
struct grpfile const *MiSTer_SelectGroup(struct grpfile const *selected);
// No GRP file: tells the player which files to copy where
void MiSTer_NoGameData(void);

// Problems the player can fix (missing data): shown on the core's purple
// screen until the player leaves the core, in plain words that say which
// files go where
void MiSTer_ShowError(const char *message);
// Exit code of the process after the player quit: the launcher starts the
// game again (back to the game list, or because the core was
// loaded again) or returns to the menu
int MiSTer_ExitCode(void);

#ifdef __cplusplus
}
#endif

#endif

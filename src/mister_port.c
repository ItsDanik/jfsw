// MiSTer hybrid core support, see mister_port.h

#include "mister_port.h"

#include <mister_hybrid.h>
#include <stdlib.h>
#include <unistd.h>

#include "build.h"
#include "cache1d.h"
#include "grpscan.h"

#define EXIT_TO_MENU 0
#define EXIT_RESTART 42 // danik_hybrid_launch.sh starts the game again

static const char *const TITLE = "jfSW";

// The player came through the list: quitting a game goes back to it
static int PickedFromList = 0;

int MiSTer_PickGame(const char *const *names, int count, int selected)
{
    int pick;

    if (!MH_Open())
        return selected;
    pick = MH_UI_Menu(TITLE, "Select a game:", names, count, selected);
    if (pick < 0 || pick >= count)
        return -1;
    PickedFromList = 1;
    MH_UI_Message(TITLE, "Loading...", 0);
    return pick;
}

void MiSTer_ShowError(const char *message)
{
    if (MH_Open())
        MH_UI_Message(TITLE, message, MH_UI_WAIT | MH_UI_ERROR);
}

void MiSTer_AddSearchPaths(void)
{
    static const char *const dirs[] = { "addons", "music", "classic/MUSIC" };
    unsigned i;

    for (i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
        if (access(dirs[i], F_OK) == 0)
            addsearchpath(dirs[i]);
    }
}

void MiSTer_Loading(void)
{
    if (MH_Open())
        MH_UI_Message(TITLE, "Loading...", 0);
}

#define MAX_GAMES 16

struct grpfile const *MiSTer_SelectGroup(struct grpfile const *selected)
{
    struct grpfile const *games[MAX_GAMES];
    const char *names[MAX_GAMES];
    struct grpfile const *grp;
    int count = 0, pick = 0;

    // only the versions and addons the game knows
    for (grp = GroupsFound(); grp && count < MAX_GAMES; grp = grp->next) {
        int i;

        if (!grp->ref)
            continue;
        // a second copy of the same file
        for (i = 0; i < count && games[i]->ref != grp->ref; i++) {
        }
        if (i < count) {
            if (grp == selected)
                selected = games[i];
            continue;
        }
        // in the order of the game's own list: the full game first
        for (i = count; i > 0 && games[i - 1]->ref > grp->ref; i--)
            games[i] = games[i - 1];
        games[i] = grp;
        count++;
    }
    for (pick = 0; pick < count; pick++) {
        names[pick] = games[pick]->ref->name;
    }
    for (pick = count - 1; pick > 0 && games[pick] != selected; pick--) {
    }
    if (count < 2)
        return selected;

    pick = MiSTer_PickGame(names, count, pick);
    if (pick < 0)
        exit(0);
    return games[pick];
}

void MiSTer_NoGameData(void)
{
    MiSTer_ShowError("No game data found.\n\n"
                     "Copy SW.GRP of Shadow Warrior to games/jfSW on the SD card.");
}

// Runs after everything else of exit(), whichever of the game's many ways out
// was taken
static void ExitWithCode(void)
{
    _exit(MiSTer_ExitCode());
}

void MiSTer_Init(void)
{
    atexit(ExitWithCode);
}

int MiSTer_ExitCode(void)
{
    // Not if we are quitting because another core was loaded
    if (PickedFromList && MH_Open())
        return EXIT_RESTART;
    return EXIT_TO_MENU;
}

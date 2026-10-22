// MiSTer hybrid core support, see mister_port.h

#include "mister_port.h"

#include <mister_hybrid.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "build.h"
#include "cache1d.h"
#include "grpscan.h"
#include "game.h"

#define EXIT_TO_MENU 0
#define EXIT_RESTART 42 // danik_hybrid_launch.sh starts the game again
#define EXIT_RELOADED 43 // the same, after the core has settled

static const char *const TITLE = "jfSW";

// OSD options of the game: CONF_STR in core/jfSW.sv, status bits 24..63

// Resolution: the video mode of the core. One the core does not have (an
// older core) is the one of 200 lines, which every core has; 320x200 as well
// when there is no core to ask.
static int ScreenMode(void)
{
    static const int option[4] = { MH_MODE_320x200, MH_MODE_640x200, MH_MODE_320x240, MH_MODE_640x240 };
    int mode;

    if (!MH_IsOpen())
        return MH_MODE_320x200;
    mode = option[MH_OSD_GAME_BITS(MH_OSDStatus(), 24, 2)];
    if (!MH_ModeAvailable(mode))
        mode = MH_ModeWidth(mode) == 640 ? MH_MODE_640x200 : MH_MODE_320x200;
    return mode;
}

int MiSTer_ScreenWidth(void)
{
    return MH_ModeWidth(ScreenMode());
}

int MiSTer_ScreenHeight(void)
{
    return MH_ModeHeight(ScreenMode());
}

int MiSTer_StickSensitivity(int stick)
{
    static const int percent[8] = { 100, 125, 150, 200, 300, 25, 50, 75 };
    const int option = percent[MH_OSD_GAME_BITS(MH_OSDStatus(), stick ? 29 : 26, 3)];

    // The whole axis walks at the speed of the keys but turns nearly four
    // times as fast as they do: 100% of the right stick is a quarter of it,
    // the turning speed of the keys when running
    return stick ? option / 4 : option;
}

int MiSTer_CheatGodMode(void)
{
    return MH_OSD_GAME_BITS(MH_OSDStatus(), 33, 1);
}

int MiSTer_CheatNoClip(void)
{
    return MH_OSD_GAME_BITS(MH_OSDStatus(), 34, 1);
}

int MiSTer_CheatTriggers(void)
{
    return MH_OSDTriggers();
}

int MiSTer_HudBottom(int lines)
{
    int top, bottom;

    MH_SetMenuMask(lines > 200 ? 0 : 1);
    if (lines <= 200)
        return 0;
    MH_HudMargins(&top, &bottom);
    return bottom * lines / 240;
}

void MiSTer_HorizLimits(int centre, int *min, int *max)
{
    // On (the default) is the bit clear
    if (!MH_OSD_GAME_BITS(MH_OSDStatus(), 32, 1)) {
        *min = centre - (centre - *min) / 2;
        *max = centre + (*max - centre) / 2;
    }
}

// The player came through the list: quitting a game goes back to it
static int PickedFromList = 0;

void MiSTer_ShowError(const char *message)
{
    if (MH_Open())
        MH_UI_Message(TITLE, message, MH_UI_WAIT | MH_UI_ERROR);
}

// Where the game looks for its files besides its folder
static const char *const SearchDirs[] = { "addons", "music", "classic/MUSIC" };
#define NUM_SEARCH_DIRS (int)(sizeof(SearchDirs) / sizeof(SearchDirs[0]))

void MiSTer_AddSearchPaths(void)
{
    int i;

    for (i = 0; i < NUM_SEARCH_DIRS; i++) {
        if (access(SearchDirs[i], F_OK) == 0)
            addsearchpath(SearchDirs[i]);
    }
}

void MiSTer_FileCase(char *name, int size)
{
    CACHE1D_FIND_REC *found;
    char *path;

    if (findfrompath(name, &path) == 0) {
        free(path);
        return;
    }
    if (strchr(name, '/') != NULL)
        return;
    found = klistpath("/", name, CACHE1D_FIND_FILE);
    if (found != NULL)
        snprintf(name, size, "%s", found->name);
    klistfree(found);
}

// The path from the game's folder of a GRP file the scan found by its name,
// in the folder or one of the search dirs; 0 if it is in none of them
static int GroupPath(const char *name, char *path, size_t size)
{
    int i;

    snprintf(path, size, "%s", name);
    for (i = 0; access(path, F_OK) != 0; i++) {
        if (i == NUM_SEARCH_DIRS)
            return 0;
        snprintf(path, size, "%s/%s", SearchDirs[i], name);
    }
    return 1;
}

void MiSTer_Loading(void)
{
    if (MH_Open())
        MH_UI_Message(TITLE, "Loading...", 0);
}

// The games with a menu entry of their own (package/games.txt), and the
// shareware that Shadow Warrior's entry starts when the full game is not
// there: the game's own kinds of GRP file (grpscan.c), which it knows by
// their checksum, whatever their name. Twin Dragon is an addon that needs
// the full game's GRP as well.
struct MiSTerGame {
    int game;
    const char *id;
    const char *name;
    const char *shareware_of;
    const char *missing;
};
static const struct MiSTerGame Games[] = {
    { GRPFILE_GAME_REG, "SW", "Shadow Warrior", NULL,
      "Shadow Warrior was not found. Copy SW.GRP of version 1.2 (the full game or the "
      "shareware) to %s/." },
    { GRPFILE_GAME_SHARE, "SWS", "Shadow Warrior Shareware", "SW", NULL },
    { GRPFILE_GAME_WD, "WT", "SW: Wanton Destruction", NULL,
      "Wanton Destruction was not found. Copy WT.GRP to %s/ or to the addons/ "
      "folder in it." },
    { GRPFILE_GAME_TD, "TD", "SW: Twin Dragon", NULL,
      "Twin Dragon was not found. It needs TD.GRP (version 1.2) and SW.GRP of the full "
      "game: copy them to %s/ (TD.GRP may go in addons/ there)." },
};
#define NUM_GAMES (int)(sizeof(Games) / sizeof(Games[0]))
#define NO_GAME "No game data found. Copy SW.GRP of Shadow Warrior (the full game or the " \
    "shareware) to %s/, and WT.GRP or TD.GRP for the addons."

#define MAX_GAMES 16

// A message with the game's folder for its %s
static void ShowMissing(const char *text)
{
    char buf[MH_MSG_ROWS * (MH_MSG_COLS + 1) + 64];

    snprintf(buf, sizeof(buf), text, MH_GamePath());
    MiSTer_ShowError(buf);
}

struct grpfile const *MiSTer_SelectGroup(struct grpfile const *selected)
{
    struct grpfile const *found[MAX_GAMES];
    struct grpfile const *grp;
    mh_game games[MAX_GAMES + NUM_GAMES];
    struct grpfile const *of[MAX_GAMES + NUM_GAMES];
    static const char *const none[] = { NULL };
    static char paths[MAX_GAMES][BMAX_PATH];
    static const char *files[MAX_GAMES][2];
    int seen[NUM_GAMES] = { 0 };
    int count = 0, n = 0, i, g, registered = 0, listed = 0, pick;

    (void)selected;
    // the versions and addons the game knows, each once, in the order of the
    // game's own list: the full game first
    for (grp = GroupsFound(); grp && count < MAX_GAMES; grp = grp->next) {
        if (!grp->ref)
            continue;
        for (i = 0; i < count && found[i]->ref != grp->ref; i++) {
        }
        if (i < count)
            continue;
        for (i = count; i > 0 && found[i - 1]->ref > grp->ref; i--)
            found[i] = found[i - 1];
        found[i] = grp;
        count++;
        if (grp->game == GRPFILE_GAME_REG)
            registered = 1;
    }
    // those, as found, their GRP file the one to look for (for the lines
    // under the game's logo: "Checking Game Data...", "Game Data OK"); an
    // addon only with the full game
    for (i = 0; i < count; i++) {
        mh_game game = { found[i]->ref->name, found[i]->ref->name, none, NULL, NULL };

        if ((found[i]->game & GRPFILE_ADDON) && !registered)
            continue;
        if (GroupPath(found[i]->name, paths[n], sizeof(paths[n]))) {
            files[n][0] = paths[n];
            files[n][1] = NULL;
            game.files = files[n];
        }
        for (g = 0; g < NUM_GAMES; g++) {
            if ((found[i]->game & ~GRPFILE_ADDON) == Games[g].game && !seen[g]) {
                seen[g] = 1;
                game.id = Games[g].id;
                game.name = Games[g].name;
                game.shareware_of = Games[g].shareware_of;
            }
        }
        of[n] = found[i];
        games[n++] = game;
    }
    // the games of a menu entry that were not found, with their message
    // (never there, whatever file names are in the folder: the shareware's
    // GRP is often called SW.GRP too)
    for (g = 0; g < NUM_GAMES; g++) {
        mh_game game = { Games[g].id, Games[g].name, MH_NOT_THERE, NULL, Games[g].missing };

        if (seen[g] || Games[g].missing == NULL)
            continue;
        of[n] = NULL;
        games[n++] = game;
    }

    pick = MH_PickGame(games, n, NULL, NO_GAME, &listed);
    if (pick < 0 || of[pick] == NULL)
        exit(0);
    PickedFromList = listed;
    // the game's logo and the lines under it ("Loading <name>..." the last)
    // come with the pick (MH_GameLogo(), host version 17)
    return of[pick];
}

void MiSTer_NoGameData(void)
{
    ShowMissing(NO_GAME);
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
    // The player loaded the core again while the game ran
    if (MH_CoreReloaded())
        return EXIT_RELOADED;
    // Not if we are quitting because another core was loaded
    if (PickedFromList && MH_Open())
        return EXIT_RESTART;
    return EXIT_TO_MENU;
}

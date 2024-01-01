// ================================================== \\
// Discord RPC implemenatation
// Based on code from the Valve Dev Community wiki
//
// Jay - 2022
// ================================================== \\

#include "hud.h"
#include "discord_manager.h"
#include <string.h>
#include "discord\discord_rpc.h"
#include <time.h>
#include <string>
#include "player_info.h"

#ifdef WIN32
#include "PlatformHeaders.h"
#else // Linux
// TODO
#endif

static DiscordRichPresence discordPresence;
extern cl_enginefunc_t gEngfuncs;

// Blank handlers; not required for singleplayer Half-Life
static void HandleDiscordReady(const DiscordUser* connectedUser) {}
static void HandleDiscordDisconnected(int errcode, const char* message) {}
static void HandleDiscordError(int errcode, const char* message) {}
static void HandleDiscordJoin(const char* secret) {}
static void HandleDiscordSpectate(const char* secret) {}
static void HandleDiscordJoinRequest(const DiscordUser* request) {}

// Default logo to use as a fallback
const char* defaultLogo = "hl";

void DiscordMan_Startup(void)
{
	// Load Fmod shared library from client folder
#ifdef WIN32
	// TODO: use built in filesystem functions to get files
	std::string gamedir = gEngfuncs.pfnGetGameDirectory();
	std::string fmod_dll_path = gamedir + "/cl_dlls/discord-rpc.dll";

	void* handle = LoadLibraryA(fmod_dll_path.c_str());

	if (!handle)
	{
		fprintf(stderr, "ERROR: Could not load discord library at %s\n", fmod_dll_path.c_str());
		return;
	}
	gEngfuncs.Con_Printf("Loaded RPC library()\n");
#else // Linux
	// TODO: Implement Linux delayed loading
#endif

	gEngfuncs.Con_Printf("DiscordMan_Startup()\n");
	int64_t startTime = time(0);

	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));

	handlers.ready = HandleDiscordReady;
	handlers.disconnected = HandleDiscordDisconnected;
	handlers.errored = HandleDiscordError;
	handlers.joinGame = HandleDiscordJoin;
	handlers.spectateGame = HandleDiscordSpectate;
	handlers.joinRequest = HandleDiscordJoinRequest;

	Discord_Initialize("1129862843508990003", &handlers, 1, 0);

	memset(&discordPresence, 0, sizeof(discordPresence));

	discordPresence.startTimestamp = startTime;
	discordPresence.largeImageKey = defaultLogo;
	Discord_UpdatePresence(&discordPresence);
}

void DiscordMan_Update(void)
{
	int index = 0;
	int players = 0;
	char curArea[64]; // If the CVar is empty, use the map file name
	//_snprintf(curArea, sizeof(curArea) - 1, strncmp(gEngfuncs.pfnGetCvarString("rpc_area"), "", sizeof(curArea)) ? gEngfuncs.pfnGetCvarString("rpc_area") : gEngfuncs.pfnGetLevelName());
	for (int i = 1; i <= 64; i++)
	{
		if (g_PlayerInfoList[i].name == nullptr) // No one uses that slot, ignore
		{
			continue;
		}
		players++;
		if (g_PlayerInfoList[i].thisplayer)
		{
			index = i;
		}
	}

	char state[64];
	get_map_name(curArea, sizeof(curArea));

	char curImage[16]; // If the CVar is empty, use the default logo
	_snprintf(curImage, sizeof(curImage) - 1, strncmp(gEngfuncs.pfnGetCvarString("rpc_image"), "", sizeof(curImage)) ? gEngfuncs.pfnGetCvarString("rpc_image") : defaultLogo);

	discordPresence.details = curArea; // Chapter name doesn't matter; if it's blank, Discord shows nothing
	discordPresence.largeImageKey = curImage;
	discordPresence.smallImageKey = "hl";
	sprintf(state, "Kills: %d Deaths: %d Players: %d", g_PlayerExtraInfo[index].frags, g_PlayerExtraInfo[index].deaths, players);
	discordPresence.state = state;

	Discord_UpdatePresence(&discordPresence);
}

void DiscordMan_Kill(void)
{
	Discord_Shutdown();
}

void DiscordMan_ExecMap(void)
{
	char command[64];
	char szCommand[256];
	get_map_name(command, sizeof(command));
	sprintf(szCommand, "exec %s.cfg\n", command);
	gEngfuncs.pfnClientCmd(szCommand);
}

static size_t get_map_name(char* dest, size_t count)
{
	auto map_path = gEngfuncs.pfnGetLevelName();

	const char* slash = strrchr(map_path, '/');
	if (!slash)
		slash = map_path - 1;

	const char* dot = strrchr(map_path, '.');
	if (!dot)
		dot = map_path + strlen(map_path);

	size_t bytes_to_copy = std::min(count - 1, static_cast<size_t>(dot - slash - 1));

	strncpy(dest, slash + 1, bytes_to_copy);
	dest[bytes_to_copy] = '\0';

	return bytes_to_copy;
}
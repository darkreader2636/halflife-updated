/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

extern void GameDLLInit( void );
void GameDLLShutdown();

#define GAMEMODE_NORMAL		0
#define GAMEMODE_VAMPIRE	1
#define GAMEMODE_KNIFEAREA	2

#define AMMO_DEFAULT	0
#define AMMO_RESERVE	1
#define AMMO_FULL		2

#define DECLARE_SKILL_CVARS(name)                 \
	cvar_t sk_##name##1 = {"sk_" #name "1", "0"}; \
	cvar_t sk_##name##2 = {"sk_" #name "2", "0"}; \
	cvar_t sk_##name##3 = {"sk_" #name "3", "0"}

#define REGISTER_SKILL_CVARS(name) \
	CVAR_REGISTER(&sk_##name##1);  \
	CVAR_REGISTER(&sk_##name##2);  \
	CVAR_REGISTER(&sk_##name##3)

extern cvar_t	displaysoundlist;

// multiplayer server rules
extern cvar_t fragsleft;
extern cvar_t timeleft;
extern cvar_t teamplay;
extern cvar_t fraglimit;
extern cvar_t timelimit;
extern cvar_t friendlyfire;
extern cvar_t falldamage;
extern cvar_t weaponstay;
extern cvar_t forcerespawn;
extern cvar_t flashlight;
extern cvar_t aimcrosshair;
extern cvar_t decalfrequency;
extern cvar_t teamlist;
extern cvar_t teamoverride;
extern cvar_t defaultteam;
extern cvar_t allowmonsters;
extern cvar_t allow_spectators;
extern cvar_t mp_chattime;

extern cvar_t sv_allowbunnyhopping;

// Deathmatch Advanced CVars
extern cvar_t spawn_health;
extern cvar_t spawn_armor;
extern cvar_t spawn_give;
extern cvar_t infinite_ammo;
extern cvar_t minus_frag;
extern cvar_t throw_crowbar;
extern cvar_t gamemode;
extern cvar_t brush_hurt;

extern cvar_t grappleairspeed;
extern cvar_t grapplewaterspeed;
extern cvar_t grappledragspeed;
extern cvar_t grappledisable;
extern cvar_t allow_grapple;

// Engine Cvars
inline cvar_t* g_psv_gravity;
inline cvar_t* g_psv_aim;
inline cvar_t* g_footsteps;
inline cvar_t* g_psv_cheats;

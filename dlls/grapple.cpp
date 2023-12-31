/*
 *	Glide Offhand Grappling Hook by Per Bernhardsson aka "Bard"
 *
 *	Modifications made for Internet play done by Will Roberson
 *
 *	Copyright (c) 1999, AfterShock Productions. All rights reserved.
 *
 *	Contact bard@ludd.luth.se or tadao@stargate.net if you have questions.
 *
========================================================================================================================================================================
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#include "grapple.h"
#include "effects.h"
#include "game.h"
#include "float.h"

// create a new entity called "grapple_bolt"

LINK_ENTITY_TO_CLASS(grapple_bolt, CGrappleBolt);

// Create the grapple

CGrappleBolt* CGrappleBolt::BoltCreate(void)
{
	// Create a new entity with CGrappleBolt private data
	CGrappleBolt* pBolt = GetClassPtr((CGrappleBolt*)NULL);
	pBolt->pev->classname = MAKE_STRING("grapple");
	pBolt->Spawn();

	return pBolt;
}

/*
 * Since I don't use the primary or secondary fire on a weapon to release
 * the grapplinghook I need some way to initialize the hook with direction
 * and velocity etc.
 */
void CGrappleBolt::Init(CBasePlayer* m_pPlayer)
{
	Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(anglesAim); // Not sure what this does. Anyone?

	anglesAim.x = -anglesAim.x;
	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 5;
	Vector vecDir = gpGlobals->v_forward;

	pev->origin = vecSrc;
	pev->angles = anglesAim;
	pev->owner = m_pPlayer->edict();

	pev->takedamage = false; // So the hook can be damaged and "killed".
	pev->health = 20;

	// The bolt doesn't move as fast under water...

	if (m_pPlayer->pev->waterlevel == 3)
	{
		pev->velocity = vecDir * grapplewaterspeed.value;
		pev->speed = grapplewaterspeed.value;
	}
	else
	{
		pev->velocity = vecDir * grapplewaterspeed.value;
		pev->speed = grapplewaterspeed.value;
	}
}

void CGrappleBolt::Spawn()
{
	if (CVAR_GET_FLOAT("allow_grapple") == 0)
		return;
	int red = 0, green = 0, blue = 0;
	Precache();
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	// make the model invisible
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 255;

	// Oz grapple glow
	// if (grappleglow.value > 0)
	//	pev->effects |= EF_LIGHT;

	red = 157;
	green = 133;
	blue = 80;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex()); // entity
	WRITE_SHORT(m_iTrail);	 // model
	WRITE_BYTE(20);			 // life
	WRITE_BYTE(2);			 // width
	WRITE_BYTE(red);		 // r, g, b
	WRITE_BYTE(green);		 // r, g, b
	WRITE_BYTE(blue);		 // r, g, b
	WRITE_BYTE(255);		 // brightness

	MESSAGE_END(); // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	pev->gravity = 0.5;

	SET_MODEL(ENT(pev), "models/crossbow_bolt.mdl");

	UTIL_SetOrigin(pev, pev->origin);
	// UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch(&CGrappleBolt::BoltTouch);	  // Call this function when something is hit.
	SetThink(&CGrappleBolt::BubbleThink); // The think function during flight.
	pev->nextthink = gpGlobals->time + 0.2;
}

/*
 * When the bolt is killed the beam should be killed also.
 * This is a virtual function so there is no need to add code
 * anywhere else for this whatever reason the bolt is killed.
 */
void CGrappleBolt::Killed(entvars_t* pevAttacker, int iGib)
{
	CBaseEntity::Killed(0, 0);
}

void CGrappleBolt::Precache()
{
	PRECACHE_MODEL("models/shell.mdl");
	PRECACHE_SOUND("weapons/xbow_fire1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod2.wav");
	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	m_iTrail = PRECACHE_MODEL("sprites/graplerope.spr"); // PRECACHE_MODEL("sprites/smoke.spr");
}


int CGrappleBolt ::Classify(void)
{
	return CLASS_NONE;
}

/*
 * This function is called when the bolt touches something.
 */
void CGrappleBolt::BoltTouch(CBaseEntity* pOther)
{
	SetTouch(NULL);
	pev->nextthink = gpGlobals->time + 0.1;

	/*
	 * Play different sounds depending on what is hit.
	 */

	entvars_t* pevOwner = VARS(pev->owner);

	CBasePlayer* plr = GetClassPtr((CBasePlayer*)pevOwner);

	if (pOther->IsPlayer())
	{

		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM);
			break;
		}

		TraceResult tr = UTIL_GetGlobalTrace();

		ClearMultiDamage();

		pOther->TraceAttack(pevOwner, 50, pev->velocity.Normalize(), &tr, DMG_NEVERGIB);

		ApplyMultiDamage(pev, pevOwner);

		plr->PlayerReleaseGrapple();
		return;
	}
	else if (pOther->IsMoving())
	{
		plr->PlayerReleaseGrapple();
		return;
	}
	else
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7));

		SetThink(&CGrappleBolt::DragPlayer); // New thinkfunction that will move the player.
		/*
		 * If the grapple hit something that is moving we need to store a
		 * reference to that entity.
		 */
		dragger = pOther->edict();
	}

	// Set up the position for the bolt.
	Vector vecDir = pev->velocity.Normalize();
	UTIL_SetOrigin(pev, pev->origin - vecDir * 12);

	/*
	 * The distance between the bolt and the dragger so we can calculate if
	 * the dragger has moved between "thinks".
	 */
	draggerDist = pev->origin - pOther->pev->origin;
	pev->angles = UTIL_VecToAngles(vecDir);
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_FLY;
	pev->velocity = Vector(0, 0, 0);
	pev->avelocity.z = 0;
	pev->angles.z = RANDOM_LONG(0, 360);

	// No sparks under water.
	if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
	{
		UTIL_Sparks(pev->origin);
	}
}

void CGrappleBolt::BubbleThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	entvars_t* pevOwner = VARS(pev->owner);
	CBaseEntity* pPlayer = GetClassPtr((CBaseEntity*)pevOwner);

	// Oz CTF - grapple colors for teams

	if (pev->waterlevel == 0)
		return;

	// If under water leave a bubbletrail.

	UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
}

void CGrappleBolt::PlayerStoppedThink(void)
{
	entvars_t* pevOwner = VARS(pev->owner);

	CBasePlayer* plr = GetClassPtr((CBasePlayer*)pevOwner);

	Vector vec_temp;

	plr->m_afPhysicsFlags |= PFLAG_ONTRAIN; // Turn on this flag so the game thinks you're on a train
											// and it can check the movement keys while hanging.

	plr->IsHanging = true;

	pevOwner->gravity = -1; // Turn off gravity

	pevOwner->movetype = MOVETYPE_NOCLIP; // Only way to make player 100% stable

	pevOwner->origin = endOrigin; // Hold this origin

	pevOwner->velocity = Vector(0, 0, 0); // Drop all velocity
	pevOwner->speed = 0;

	// UTIL_SetOrigin( pevOwner, endOrigin );



	pev->nextthink = gpGlobals->time;
}

void CGrappleBolt::DragPlayer()
{
	pev->nextthink = gpGlobals->time + 0.1;
	if (pev->owner)
	{
		entvars_t* pevOwner = VARS(pev->owner);
		CBasePlayer* plr = GetClassPtr((CBasePlayer*)pevOwner);

		pevOwner->gravity = -1;

		Vector prevOrigin = pev->origin;

		//	pev->origin = VARS(dragger)->origin + draggerDist;

		/*
		 * If the origin has changed, make sure that the grapple
		 * continues to drag.
		 */

		Vector dist = (pev->origin - pevOwner->origin);

		if (dist.Length() > 50)
		{
			pevOwner->velocity = dist.Normalize() * grappledragspeed.value;
			pevOwner->speed = grappledragspeed.value;
		}
		else
		{
			pevOwner->velocity = Vector(0, 0, 0);
			pevOwner->speed = 0;
			endOrigin = pevOwner->origin;
			SetThink(&CGrappleBolt::PlayerStoppedThink);
		}
	}
}
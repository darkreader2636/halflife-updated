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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "UserMessages.h"


LINK_ENTITY_TO_CLASS(weapon_sniperrifle, CSniperRifle);

void CSniperRifle::Precache()
{
	pev->classname = MAKE_STRING("weapon_sniperrifle");

	BaseClass::Precache();

	m_iId = WEAPON_SNIPERRIFLE;

	PRECACHE_MODEL("models/w_m40a1.mdl");
	PRECACHE_MODEL("models/v_m40a1.mdl");
	PRECACHE_MODEL("models/p_m40a1.mdl");

	PRECACHE_SOUND("weapons/sniper_fire.wav");
	PRECACHE_SOUND("weapons/sniper_zoom.wav");
	PRECACHE_SOUND("weapons/sniper_reload_first_seq.wav");
	PRECACHE_SOUND("weapons/sniper_reload_second_seq.wav");
	PRECACHE_SOUND("weapons/sniper_miss.wav");
	PRECACHE_SOUND("weapons/sniper_bolt1.wav");
	PRECACHE_SOUND("weapons/sniper_bolt2.wav");

	m_usSniper = PRECACHE_EVENT(1, "events/sniper.sc");
}

void CSniperRifle::Spawn()
{
	Precache();

	SET_MODEL(edict(), "models/w_m40a1.mdl");

	m_iDefaultAmmo = SNIPERRIFLE_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.
}

bool CSniperRifle::Deploy()
{
	return BaseClass::DefaultDeploy("models/v_m40a1.mdl", "models/p_m40a1.mdl", SNIPERRIFLE_DRAW, "bow");
}

void CSniperRifle::Holster()
{
	m_fInReload = false; // cancel any reload in progress.

	if (m_pPlayer->m_iFOV != 0)
	{
		SecondaryAttack();
	}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.25;

	SendWeaponAnim(SNIPERRIFLE_HOLSTER);
}

void CSniperRifle::WeaponIdle()
{
	// Update autoaim
	m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);

	ResetEmptySound();

	if (m_bReloading && gpGlobals->time >= m_flReloadStart + 2.324)
	{
		SendWeaponAnim(SNIPERRIFLE_RELOAD2);
		m_bReloading = false;
	}

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (0 != m_iClip)
			SendWeaponAnim(SNIPERRIFLE_SLOWIDLE);
		else
			SendWeaponAnim(SNIPERRIFLE_SLOWIDLE2);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.348;
	}
}

void CSniperRifle::PrimaryAttack()
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
		return;
	}

	if (0 == m_iClip)
	{
		PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	UseClip(1);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

	UTIL_MakeVectors(vecAngles);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);

	// TODO: 8192 constant should be defined somewhere - Solokiller
	Vector vecShot = m_pPlayer->FireBulletsPlayer(1,
		vecSrc, vecAiming, g_vecZero,
		8192, BULLET_PLAYER_762, 0, 0,
		m_pPlayer->pev, m_pPlayer->random_seed);

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags,
		m_pPlayer->edict(), m_usSniper, 0,
		g_vecZero, g_vecZero,
		vecShot.x, vecShot.y,
		m_iClip, m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()],
		0, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.0f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;
}

void CSniperRifle::SecondaryAttack()
{
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/sniper_zoom.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	ToggleZoom();

	// TODO: this doesn't really make sense
	pev->nextthink = 0.1;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CSniperRifle::Reload()
{
	if (m_pPlayer->ammo_762 > 0)
	{
		if (m_pPlayer->m_iFOV != 0)
		{
			ToggleZoom();
		}

		if (0 != m_iClip)
		{
			if (DefaultReload(SNIPERRIFLE_MAX_CLIP, SNIPERRIFLE_RELOAD3, 2.324))
			{
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.324;
			}
		}
		else if (DefaultReload(SNIPERRIFLE_MAX_CLIP, SNIPERRIFLE_RELOAD1, 2.324))
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 4.102;
			m_flReloadStart = gpGlobals->time;
			m_bReloading = true;
		}
		else
		{
			m_bReloading = false;
		}
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.102;
}

int CSniperRifle::iItemSlot()
{
	return 4;
}

bool CSniperRifle::GetItemInfo(ItemInfo* p)
{
	p->pszAmmo1 = "762";
	p->iMaxAmmo1 = SNIPERRIFLE_MAX_CARRY;
	p->pszName = STRING(pev->classname);
	p->pszAmmo2 = 0;
	p->iMaxAmmo2 = WEAPON_NOCLIP;
	p->iMaxClip = SNIPERRIFLE_MAX_CLIP;
	p->iSlot = 5;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SNIPERRIFLE;
	p->iWeight = SNIPERRIFLE_WEIGHT;
	return true;
}

void CSniperRifle::IncrementAmmo(CBasePlayer* pPlayer)
{
	if (pPlayer->GiveAmmo(1, "762", SNIPERRIFLE_MAX_CARRY) >= 0)
	{
		EMIT_SOUND(pPlayer->edict(), CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM);
	}
}

void CSniperRifle::ToggleZoom()
{
	if (m_pPlayer->m_iFOV == 0)
	{
		m_pPlayer->m_iFOV = 18;
	}
	else
	{
		m_pPlayer->m_iFOV = 0;
	}
}

class CSniperRifleAmmo : public CBasePlayerAmmo
{
public:
	using BaseClass = CBasePlayerAmmo;

	void Spawn() override
	{
		Precache();
		SET_MODEL(edict(), "models/w_m40a1clip.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/w_m40a1clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_SNIPERRIFLE_GIVE, "762", SNIPERRIFLE_MAX_CARRY) != -1)
		{
			EMIT_SOUND(edict(), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);

			return true;
		}

		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_762, CSniperRifleAmmo);

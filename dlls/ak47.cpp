#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"

LINK_ENTITY_TO_CLASS(weapon_ak47, CAK47)

void CAK47::Spawn()
{
	Precache();

	m_iId = WEAPON_AK47;
	SET_MODEL(edict(), "models/w_ak47.mdl");

	m_iDefaultAmmo = AK47_DEFAULT_GIVE;

	// Get ready to fall down
	FallInit();

	// extend
	CBasePlayerWeapon::Spawn();
}

void CAK47::Precache()
{
	PRECACHE_MODEL("models/v_ak47.mdl");
	PRECACHE_MODEL("models/w_ak47.mdl");
	PRECACHE_MODEL("models/p_ak47.mdl");

	PRECACHE_SOUND("weapons/ak47-1.wav");
	PRECACHE_SOUND("weapons/ak47-2.wav");
	PRECACHE_SOUND("weapons/ak47_clipout.wav");
	PRECACHE_SOUND("weapons/ak47_clipin.wav");
	PRECACHE_SOUND("weapons/ak47_boltpull.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireAK47 = PRECACHE_EVENT(1, "events/ak47.sc");
}

bool CAK47::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = AK47_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 5;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_AK47;
	p->iFlags = 0;
	p->iWeight = AK47_WEIGHT;

	return true;
}

bool CAK47::Deploy()
{
	iShellOn = 1;

	return DefaultDeploy("models/v_ak47.mdl", "models/p_ak47.mdl", AK47_DRAW, "ak47", UseDecrement() != false);
}

void CAK47::SecondaryAttack()
{

}

void CAK47::PrimaryAttack()
{
	Vector vecAiming, vecSrc, vecDir;
	int flag;

	m_iShotsFired++;

	float m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200.0f) + 0.35f;

	if (m_flAccuracy > 1.25f)
		m_flAccuracy = 1.25f;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = GetNextAttackDelay(0.2);
		}

		return;
	}

	UseClip(1);
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	float flBaseDamage = 36.0f;

	//vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_762, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);

		// vecDir = m_pPlayer->FireBullets(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, 2, BULLET_PLAYER_762, flBaseDamage, AK47_RANGE_MODIFER, m_pPlayer->pev, false, m_pPlayer->random_seed);
	if (m_pPlayer->m_afButtonLast & IN_ATTACK)
	{
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_762, 2, 18, m_pPlayer->pev, m_pPlayer->random_seed);
	}
	else
	{
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_762, 2, 18, m_pPlayer->pev, m_pPlayer->random_seed);
	}

#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif

	PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireAK47, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y,
		int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), false, false);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(0.0955f);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", SUIT_SENTENCE, SUIT_REPEAT_OK);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9f;
}

void CAK47::Reload()
{
	// to prevent reload if not enough ammo
	if (m_pPlayer->ammo_762nato <= 0)
		return;

	if (DefaultReload(iMaxClip(), AK47_RELOAD, 2.45f))
	{
		m_iShotsFired = 0;
	}
	return;
}

void CAK47::WeaponIdle()
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0f;
		SendWeaponAnim(AK47_IDLE1, UseDecrement() != false);
	}
}

int CAK47::iItemSlot()
{
	return 4;
}

class CAK47Ammo : public CBasePlayerAmmo
{
public:
	using BaseClass = CBasePlayerAmmo;
	void Spawn() override
	{
		Precache();
		SET_MODEL(edict(), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_AK47_GIVE, "762nato", AK47_MAX_CARRY) != -1)
		{
			EMIT_SOUND(edict(), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);

			return true;
		}

		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_762nato, CAK47Ammo);
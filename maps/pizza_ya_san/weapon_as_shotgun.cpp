#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"

/*
 * HLベース改造ショットガン
 *
 * fgdファイルに下記を追加して、エンティティとしてマップに仕込むこと。
 * @PointClass base(Weapon, Targetx, ExclusiveHold) studio("models/pizza_ya_san/w_shotgun_shorty.mdl") = weapon_as_shotgun : "custom shotgun" []
 */
const Vector VECTOR_CONE_DM_SHOTGUN(0.13074, 0.13074, 0.00);		// 15 degrees

#define AS_SHOTGUN_DEFAULT_AMMO 12
#define AS_SHOTGUN_MAX_CARRY 125
#define AS_SHOTGUN_MAX_CLIP 4
#define AS_SHOTGUN_WEIGHT 15

#define AS_SHOTGUN_PELLETCOUNT 9

ItemInfo g_shotgun_info = {
	2,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"buckshot",						// pszAmmo1
	BUCKSHOT_MAX_CARRY,				// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"pizza_ya_san/weapon_as_shotgun",// pszName (path to HUD config)
	AS_SHOTGUN_MAX_CLIP,			// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	AS_SHOTGUN_WEIGHT				// iWeight
};

enum ShotgunAnimation
{
	SHOTGUN_IDLE = 0,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};

class CPizzaShotgun : public CBasePlayerWeapon
{
	float m_flNextReload;
	int m_iShell;
	float m_flPumpTime;
	bool m_fPlayPumpSound;
	bool m_fReloading;

	void Spawn()
	{
		Precache();
		SET_MODEL(edict(), "models/pizza_ya_san/w_shotgun_shorty.mdl");

		m_iDefaultAmmo = AS_SHOTGUN_DEFAULT_AMMO;
		m_iId = g_shotgun_info.iId;

		FallInit();// get ready to fall
	}

	void Precache()
	{
		m_defaultModelV = "models/pizza_ya_san/v_shotgun_shorty.mdl";
		m_defaultModelP = "models/pizza_ya_san/p_shotgun_shorty.mdl";
		m_defaultModelW = "models/pizza_ya_san/w_shotgun_shorty.mdl";
		CBasePlayerWeapon::Precache();

		m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");// shotgun shell

		PRECACHE_SOUND("items/9mmclip1.wav");

		PRECACHE_SOUND("weapons/dbarrel1.wav");//shotgun
		PRECACHE_SOUND("weapons/sbarrel1.wav");//shotgun

		PRECACHE_SOUND("weapons/reload1.wav");	// shotgun reload
		PRECACHE_SOUND("weapons/reload3.wav");	// shotgun reload

		PRECACHE_SOUND("weapons/sshell1.wav");	// shotgun reload
		PRECACHE_SOUND("weapons/sshell3.wav");	// shotgun reload

		PRECACHE_SOUND("weapons/357_cock1.wav"); // gun empty sound
		PRECACHE_SOUND("weapons/scock1.wav");	// cock gun

		PRECACHE_GENERIC("sprites/pizza_ya_san/weapon_as_shotgun.txt");
	}

	int AddToPlayer(CBasePlayer* pPlayer)
	{
		if (CBasePlayerWeapon::AddToPlayer(pPlayer))
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, m_hPlayer->pev);
			WRITE_BYTE(m_iId);
			MESSAGE_END();
			return TRUE;
		}
		return FALSE;
	}

	int GetItemInfo(ItemInfo* p)
	{
		*p = g_shotgun_info;
		return 1;
	}

	BOOL Deploy()
	{
		return DefaultDeploy(GetModelV(), GetModelP(), SHOTGUN_DRAW, "shotgun");
	}

	void ItemPostFrame()
	{
		if (m_flPumpTime != 0 && m_flPumpTime < gpGlobals->time && m_fPlayPumpSound)
		{
			// play pumping sound
			EMIT_SOUND_DYN(m_hPlayer->edict(), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 0x1f));

			m_fPlayPumpSound = false;
		}

		CBasePlayerWeapon::ItemPostFrame();
	}

	void CreatePelletDecals(const Vector& vecSrc, const Vector& vecAiming, const Vector& vecSpread, const uint32_t uiPelletCount)
	{
		TraceResult tr;

		for (uint32_t uiPellet = 0; uiPellet < uiPelletCount; ++uiPellet)
		{
			// get circular gaussian spread
			float x, y, z;
			do {
				x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);

			Vector vecDir = vecAiming
				+ x * vecSpread.x * gpGlobals->v_right
				+ y * vecSpread.y * gpGlobals->v_up;

			Vector vecEnd = vecSrc + vecDir * 2048;

			TRACE_LINE(vecSrc, vecEnd, dont_ignore_monsters, m_hPlayer->edict(), &tr);

			if (tr.flFraction < 1.0)
			{
				if (tr.pHit)
				{
					CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

					if (!pHit || pHit->IsBSPModel())
						DecalGunshot(&tr, BULLET_PLAYER_BUCKSHOT);
				}
			}
		}
	}

	void PrimaryAttack()
	{
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		// don't fire underwater
		if (m_hPlayer->pev->waterlevel == WATERLEVEL_HEAD)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->time + 0.15;
			return;
		}

		if (m_iClip <= 0)
		{
			m_flNextPrimaryAttack = m_flTimeWeaponIdle = gpGlobals->time + 0.75;
			Reload();
			PlayEmptySound();
			return;
		}

		SendWeaponAnim(SHOTGUN_FIRE, 0, 0);

		EMIT_SOUND_DYN(m_hPlayer->edict(), CHAN_WEAPON, "weapons/sbarrel1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0x1f));

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		--m_iClip;

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		m_pPlayer->FireBullets(AS_SHOTGUN_PELLETCOUNT, vecSrc, vecAiming, VECTOR_CONE_DM_SHOTGUN, 2048, BULLET_PLAYER_BUCKSHOT, 0);

		//Shell ejection
		MAKE_VECTORS(m_pPlayer->pev->v_angle);

		Vector	vecShellVelocity = m_pPlayer->pev->velocity
			+ gpGlobals->v_right * RANDOM_FLOAT(50, 70)
			+ gpGlobals->v_up * RANDOM_FLOAT(100, 150)
			+ gpGlobals->v_forward * 25;

		EjectBrass(vecSrc + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -34 + gpGlobals->v_forward * 14 + gpGlobals->v_right * 6, vecShellVelocity, m_pPlayer->pev->angles.y, m_iShell, TE_BOUNCE_SHELL);

		if (m_iClip == 0 && m_pPlayer->rgAmmo(m_iPrimaryAmmoType) <= 0)
			// HEV suit - indicate out of ammo condition
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

		if (m_iClip != 0)
			m_flPumpTime = gpGlobals->time + 0.5;

		m_pPlayer->pev->punchangle.x = -5.0;

		m_flNextPrimaryAttack = gpGlobals->time + 0.65;
		m_flNextSecondaryAttack = gpGlobals->time + 0.65;

		if (m_iClip != 0)
			m_flTimeWeaponIdle = gpGlobals->time + 5.0;
		else
			m_flNextPrimaryAttack = m_flTimeWeaponIdle = gpGlobals->time + 0.75;

		m_fReloading = false;
		m_fPlayPumpSound = true;

		CreatePelletDecals(vecSrc, vecAiming, VECTOR_CONE_DM_SHOTGUN, AS_SHOTGUN_PELLETCOUNT);
	}

	void SecondaryAttack()
	{
		PrimaryAttack();
		/*
		SendWeaponAnim(SHOTGUN_FIRE2, 0, 0);
		m_flNextPrimaryAttack = gpGlobals->time + 1.7;
		m_flNextSecondaryAttack = gpGlobals->time + 1.7;
		m_flTimeWeaponIdle = gpGlobals->time + 1.0;
		m_fReloading = true;
		m_fPlayPumpSound = false;
		*/
	}

	void Holster(int skiplocal)
	{
		m_fReloading = false;

		CBasePlayerWeapon::Holster();
	}

	void Reload()
	{
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		if (m_pPlayer->rgAmmo(m_iPrimaryAmmoType) <= 0 || m_iClip == AS_SHOTGUN_MAX_CLIP)
			return;

		if (m_flNextReload > gpGlobals->time)
			return;

		// don't reload until recoil is done
		if (m_flNextPrimaryAttack > gpGlobals->time && !m_fReloading)
			return;

		// check to see if we're ready to reload
		if (!m_fReloading)
		{
			SendWeaponAnim(SHOTGUN_START_RELOAD, 0, 0);
			m_pPlayer->m_flNextAttack = 0.3;
			m_flTimeWeaponIdle = gpGlobals->time + 0.3;
			m_flNextPrimaryAttack = gpGlobals->time + 0.5;
			m_flNextSecondaryAttack = gpGlobals->time + 0.5;
			m_fReloading = true;
			return;
		}
		else if (m_fReloading)
		{
			if (m_flTimeWeaponIdle > gpGlobals->time)
				return;

			if (m_iClip == AS_SHOTGUN_MAX_CLIP)
			{
				m_fReloading = false;
				return;
			}

			SendWeaponAnim(SHOTGUN_RELOAD, 0);
			m_flNextReload = gpGlobals->time + 0.25;
			m_flNextPrimaryAttack = gpGlobals->time + 0.25;
			m_flNextSecondaryAttack = gpGlobals->time + 0.25;
			m_flTimeWeaponIdle = gpGlobals->time + 0.25;


			// Add them to the clip
			m_iClip += 1;
			m_pPlayer->rgAmmo(m_iPrimaryAmmoType, m_pPlayer->rgAmmo(m_iPrimaryAmmoType) - 1);

			int r = RANDOM_LONG(0, 1);
			switch (r)
			{
			case 0:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));
				break;
			case 1:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/reload3.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));
				break;
			}
		}

		CBasePlayerWeapon::Reload();
	}

	void WeaponIdle()
	{
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		ResetEmptySound();

		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flTimeWeaponIdle < gpGlobals->time)
		{
			if (m_iClip == 0 && !m_fReloading && m_pPlayer->rgAmmo(m_iPrimaryAmmoType) != 0)
			{
				Reload();
			}
			else if (m_fReloading)
			{
				if (m_iClip != AS_SHOTGUN_MAX_CLIP && m_pPlayer->rgAmmo(m_iPrimaryAmmoType) > 0)
				{
					Reload();
				}
				else
				{
					// reload debounce has timed out
					SendWeaponAnim(SHOTGUN_PUMP, 0, 0);

					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 0x1f));
					m_fReloading = false;
					m_flTimeWeaponIdle = gpGlobals->time + 1.5;
				}
			}
			else
			{
				int iAnim;
				float flRand = RANDOM_FLOAT(0, 1);
				if (flRand <= 0.8)
				{
					iAnim = SHOTGUN_IDLE_DEEP;
					m_flTimeWeaponIdle = gpGlobals->time + (60.0 / 12.0);// * RANDOM_LONG(2, 5);
				}
				else if (flRand <= 0.95)
				{
					iAnim = SHOTGUN_IDLE;
					m_flTimeWeaponIdle = gpGlobals->time + (20.0 / 9.0);
				}
				else
				{
					iAnim = SHOTGUN_IDLE4;
					m_flTimeWeaponIdle = gpGlobals->time + (20.0 / 9.0);
				}
				SendWeaponAnim(iAnim, 1, 0);
			}
		}
	}
};

LINK_ENTITY_TO_CLASS(weapon_as_shotgun, CPizzaShotgun);

#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"

/*
 * Jetpack & Glock18
 * (Refeneced: The original Half-Life version of the mp5)
 */
enum GlockAnimation {
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

const int JETPACK_DEFAULT_GIVE = 100;
const int JETPACK_MAX_AMMO = 250;
const int JETPACK_MAX_AMMO2 = 100;
const int JETPACK_MAX_CLIP = 10;
const int JETPACK_WEIGHT = 5;

const float LOW_GRAVITY = 0.3;
const float NORMAL_GRAVITY = 1.0;

const int FUEL_CYCLE = 60;

ItemInfo g_jetpack_info = {
	2,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	JETPACK_MAX_AMMO,				// iMaxAmmo1
	"uranium",						// pszAmmo2
	JETPACK_MAX_AMMO2,				// iMaxAmmo2
	"pizza_ya_san/weapon_as_jetpack",// pszName (path to HUD config)
	JETPACK_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	JETPACK_WEIGHT					// iWeight
};

class CJetpack : public CBasePlayerWeapon {
	float m_flNextAnimTime;
	int m_iShell;
	int m_iSecondaryAmmo;
	int m_iFuel; // 燃料
	// 爆発＆スモーク用
	int m_iBurnSound;
	int m_gBurnSprite;
	int m_gSmokeSprite;

	float m_gravityLowTime; // 重力軽減時間用

	void Spawn() {
		Precache();
		SET_MODEL(edict(), "models/pizza_ya_san/w_glock18jet.mdl");

		m_iDefaultAmmo = JETPACK_DEFAULT_GIVE;
		m_iSecondaryAmmoType = 0;
		m_iId = g_jetpack_info.iId;
		FallInit();
		m_iClip = 3;
	}

	void Precache() {
		m_defaultModelV = "models/pizza_ya_san/v_glock18jet.mdl";
		m_defaultModelP = "models/pizza_ya_san/p_glock18jet.mdl";
		m_defaultModelW = "models/pizza_ya_san/w_glock18jet.mdl";
		CBasePlayerWeapon::Precache();

		m_iShell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_MODEL("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");

		//These are played by the model, needs changing there
		PRECACHE_SOUND("hl/items/clipinsert1.wav");
		PRECACHE_SOUND("hl/items/cliprelease1.wav");
		PRECACHE_SOUND("hl/items/guncock1.wav");

		PRECACHE_SOUND("/weapons/hks1.wav");
		PRECACHE_SOUND("hl/weapons/357_cock1.wav");

		// 噴射サウンド
		PRECACHE_SOUND("ambience/steamburst1.wav");
		// 噴射
		m_gBurnSprite = PRECACHE_MODEL("sprites/xflare2.spr");
		m_gSmokeSprite = PRECACHE_MODEL("sprites/boom3.spr");
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_jetpack_info;
		return true;
	}

	int AddToPlayer(CBasePlayer* pPlayer) {
		if (CBasePlayerWeapon::AddToPlayer(pPlayer))
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, m_hPlayer->pev);
			WRITE_BYTE(m_iId);
			MESSAGE_END();
			return TRUE;
		}

		return FALSE;
	}

	BOOL Deploy() {
		m_iFuel = 0;
		m_iBurnSound = 0;
		return DefaultDeploy(GetModelV(), GetModelP(), GLOCK_DRAW, "onehanded");
	}

	void Holster(int skiplocal) {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		m_pPlayer->pev->gravity = NORMAL_GRAVITY;
	}

	float WeaponTimeBase() {
		return gpGlobals->time; //g_WeaponFuncs.WeaponTimeBase();
	}

	// プライマリアタック
	void PrimaryAttack() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		// 水中は射撃不可、弾薬なし
		if ((m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD)
			|| (m_pPlayer->rgAmmo(m_iPrimaryAmmoType) <= 0)
			) {
			PlayEmptySound();
			m_flNextPrimaryAttack = WeaponTimeBase() + 0.15;
			return;
		}

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		m_pPlayer->rgAmmo(m_iPrimaryAmmoType, m_pPlayer->rgAmmo(m_iPrimaryAmmoType) - 1);

		// 弾薬があるなら
		if (m_pPlayer->rgAmmo(m_iPrimaryAmmoType) > 1) {
			SendWeaponAnim(GLOCK_SHOOT, 0, 0);
		}
		else {
			SendWeaponAnim(GLOCK_SHOOT_EMPTY, 0, 0);
		}

		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "/weapons/hks1.wav", 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));

		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		// 弾丸の処理
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
		m_pPlayer->FireBullets(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_MP5, 2);

		// 薬莢
		MAKE_VECTORS(m_pPlayer->pev->v_angle);

		Vector vecShellVelocity = m_pPlayer->pev->velocity
			+ gpGlobals->v_right * RANDOM_FLOAT(50, 70)
			+ gpGlobals->v_up * RANDOM_FLOAT(100, 150)
			+ gpGlobals->v_forward * 25;
		EjectBrass(vecSrc
			+ m_pPlayer->pev->view_ofs
			+ gpGlobals->v_up * -34
			+ gpGlobals->v_forward * 14 + gpGlobals->v_right * 6,
			vecShellVelocity,
			m_pPlayer->pev->angles.y,
			m_iShell,
			TE_BOUNCE_SHELL);

		m_pPlayer->pev->punchangle.x = RANDOM_LONG(-2, 2);

		m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.05;
		if (m_flNextPrimaryAttack < WeaponTimeBase()) {
			m_flNextPrimaryAttack = WeaponTimeBase() + 0.05;
		}

		m_flTimeWeaponIdle = WeaponTimeBase() + RANDOM_FLOAT(10, 15);

		TraceResult tr;
		float x, y;
		GetCircularGaussianSpread(x, y);

		Vector vecDir = vecAiming
			+ x * VECTOR_CONE_6DEGREES.x * gpGlobals->v_right
			+ y * VECTOR_CONE_6DEGREES.y * gpGlobals->v_up;

		Vector vecEnd = vecSrc + vecDir * 4096;

		TRACE_LINE(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0) {
			if (tr.pHit) {
				CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
				if (!pHit || pHit->IsBSPModel()) {
					DecalGunshot(&tr, BULLET_PLAYER_MP5);
				}
			}
		}
	}

	// セカンダリアタック
	void SecondaryAttack() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		// 水中は飛行不可、また燃料なしは飛ばない
		if ((m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD)
			|| (m_pPlayer->rgAmmo(m_iSecondaryAmmoType) <= 0)) {
			PlayEmptySound();
			m_flNextSecondaryAttack = WeaponTimeBase() + 0.15;
			return;
		}

		m_pPlayer->pev->gravity = LOW_GRAVITY;
		m_gravityLowTime = WeaponTimeBase();

		m_flNextSecondaryAttack = WeaponTimeBase() + 0.01;

		// 加速
		float accX = m_pPlayer->pev->velocity.x;
		accX = (accX < 10) ? 0 : 0.15;
		float accY = m_pPlayer->pev->velocity.y;
		accY = (accY < 10) ? 0 : 0.15;

		// clipをPowerLevelにしている。つまり、これによって出力調整
		m_pPlayer->pev->velocity = m_pPlayer->pev->velocity
			+ 10 * Vector(accX, accY, 1)
			+ (3 * m_iClip) * Vector(0, 0, 1);

		// 燃料減
		m_iFuel++;
		m_iFuel %= FUEL_CYCLE;
		if (m_iFuel == 1) {
			m_pPlayer->rgAmmo(m_iSecondaryAmmoType, m_pPlayer->rgAmmo(m_iSecondaryAmmoType) - 1);
		}

		// 発射音＆スプライト
		m_iBurnSound++;
		m_iBurnSound %= (FUEL_CYCLE / 2);
		if (m_iBurnSound == 1) {
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON,
				"ambience/steamburst1.wav", 0.8, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));

			uint8 scale = 150;

			// 爆発
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(m_pPlayer->pev->origin.x);
			WRITE_COORD(m_pPlayer->pev->origin.y);
			WRITE_COORD(m_pPlayer->pev->origin.z);
			WRITE_SHORT(m_gBurnSprite);
			WRITE_BYTE(15); // sacle
			WRITE_BYTE(50); // framerate
			WRITE_BYTE(4);  // flag 1=不透明、2=発光なし、4=音なし、8=パーティクルなし
			MESSAGE_END();

			// 煙
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(m_pPlayer->pev->origin.x);
			WRITE_COORD(m_pPlayer->pev->origin.y);
			WRITE_COORD(m_pPlayer->pev->origin.z);
			WRITE_SHORT(m_gSmokeSprite);
			WRITE_BYTE(scale);    // scale
			WRITE_BYTE(10); // framerate
			MESSAGE_END();

		}
	}

	void Reload() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		// リロードといいつつ、出力レベル調整。clipが出力レベル
		if ((WeaponTimeBase() > m_flNextPrimaryAttack)
			&& (WeaponTimeBase() > m_flNextSecondaryAttack)
			) {
			// しゃがみ中で-1、立ち状態で+1
			int pitch = 100;
			if ((m_pPlayer->pev->button & IN_DUCK) != 0) {
				m_iClip--;
				m_iClip = (m_iClip < 1) ? JETPACK_MAX_CLIP : m_iClip;
				pitch = 90;
			}
			else {
				m_iClip++;
				m_iClip = (m_iClip > JETPACK_MAX_CLIP) ? 1 : m_iClip;
				pitch = 110;
			}
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "hl/weapons/357_cock1.wav", 1.0, ATTN_NORM, 0, pitch);

			m_flNextPrimaryAttack = WeaponTimeBase() + 0.5;
			m_flNextSecondaryAttack = WeaponTimeBase() + 0.5;

			UTIL_ClientPrint(m_pPlayer->edict(), HUD_PRINTCENTER, UTIL_VarArgs("Boost Level: %d", m_iClip));
		}
	}

	void WeaponIdle() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		if (WeaponTimeBase() > m_gravityLowTime + 1.0) {
			m_pPlayer->pev->gravity = NORMAL_GRAVITY;
		}

		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flTimeWeaponIdle > WeaponTimeBase()) {
			return;
		}

		int iAnim;
		int r = RANDOM_LONG(0, 1);
		switch (r) {
		case 0:  iAnim = GLOCK_IDLE1; break;
		case 1:  iAnim = GLOCK_IDLE2; break;
		default: iAnim = GLOCK_IDLE3; break;
		}

		SendWeaponAnim(iAnim);
		m_flTimeWeaponIdle = WeaponTimeBase() + RANDOM_FLOAT(10, 15);// how long till we do this again.
	}
};


LINK_ENTITY_TO_CLASS(weapon_as_jetpack, CJetpack);
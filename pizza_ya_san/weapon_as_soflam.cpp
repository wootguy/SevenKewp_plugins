#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"

/*
 * SOFLAM - AirStrike
* (** Refeneced: Nero's AirStrike plugin)
 */
enum SoflamAnimation {
	SOFLAM_IDLE1 = 0,
	SOFLAM_DRAW,
	SOFLAM_SHOOT,
	SOFLAM_SHOOT2
};

const int SOFLAM_DEFAULT_GIVE = 5;
const int SOFLAM_MAX_AMMO = 5;
const int SOFLAM_MAX_CLIP = -1;
const int SOFLAM_WEIGHT = 5;

const float SOFLAM_DELAY = 15.0;    // 爆撃後の再攻撃可能時間
const float STRIKE_CNT = 8;         // 砲弾数
const float STRIKE_INTERVAL = 0.75; // 砲弾落下感覚
const float STRIKE_BEFORE = 5.0;    // 要請後のDelay
const float BONUS_RANGE = 120.0;

ItemInfo g_soflam_info = {
	1,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"rockets",						// pszAmmo1
	SOFLAM_MAX_AMMO,				// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"pizza_ya_san/weapon_as_soflam",// pszName (path to HUD config)
	SOFLAM_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	SOFLAM_WEIGHT					// iWeight
};

class CSoflam : public CBasePlayerWeapon {
	float m_flNextAnimTime;
	int m_gDotSprite;   // レーザーポインタ
	int m_gLaserSprite; // レーザー
	float m_lastRequested;

	int m_strikeStatus = -1;
	Vector m_strikeAbovePos = Vector(0, 0, 0);
	Vector m_strikeHitPos = Vector(0, 0, 0);
	float m_lastStrike = 0;
	int m_strikeType = 0;


	void Spawn() {
		Precache();
		SET_MODEL(edict(), "models/pizza_ya_san/w_soflam.mdl");
		m_iDefaultAmmo = SOFLAM_DEFAULT_GIVE;
		m_iId = g_soflam_info.iId;
		FallInit();
	}

	void Precache() {
		m_defaultModelV = "models/pizza_ya_san/v_soflam.mdl";
		m_defaultModelP = "models/pizza_ya_san/p_soflam.mdl";
		m_defaultModelW = "models/pizza_ya_san/w_soflam.mdl";
		CBasePlayerWeapon::Precache();

		PRECACHE_MODEL("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
		PRECACHE_SOUND("hl/weapons/357_cock1.wav");

		// mortar
		PRECACHE_MODEL("models/mortarshell.mdl");
		PRECACHE_SOUND("weapons/ofmortar.wav");

		// ラジオ用
		PRECACHE_SOUND("hgrunt/yessir.wav");
		PRECACHE_SOUND("hgrunt/affirmative.wav");
		PRECACHE_SOUND("hgrunt/roger.wav");
		PRECACHE_SOUND("hgrunt/negative.wav");

		// レーザーポインタ
		m_gDotSprite = PRECACHE_MODEL("sprites/red.spr");
		m_gLaserSprite = PRECACHE_MODEL("sprites/laserbeam.spr");
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_soflam_info;
		return true;
	}

	int AddToPlayer(CBasePlayer* pPlayer) {
		if (!CBasePlayerWeapon::AddToPlayer(pPlayer))
			return false;

		 if (CBasePlayerWeapon::AddToPlayer(pPlayer))
		 {
			 MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->edict());
			 WRITE_BYTE(g_soflam_info.iId);
			 MESSAGE_END();
			 return TRUE;
		 }
		return true;
	}

	BOOL Deploy() {
		m_strikeStatus = -1;
		m_lastRequested = 0.0;
		m_flNextPrimaryAttack = WeaponTimeBase() + 1.0;
		m_flNextSecondaryAttack = WeaponTimeBase() + 1.0;
		return DefaultDeploy(GetModelV(), GetModelP(), SOFLAM_DRAW, "trip");
	}

	float WeaponTimeBase() {
		return gpGlobals->time; //g_WeaponFuncs.WeaponTimeBase();
	}

	// プライマリアタック
	void PrimaryAttack() {
		AirStrikeAttack(0);
	}

	// セカンダリアタック
	void SecondaryAttack() {
		AirStrikeAttack(1);
	}

	void AirStrikeAttack(int strikeType) {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		// No Ammo なら終了
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0) {
			PlayEmptySound();
			m_flNextPrimaryAttack = WeaponTimeBase() + 0.15;
			m_flNextSecondaryAttack = WeaponTimeBase() + 0.15;
			return;
		}

		// 攻撃モーション
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		if (strikeType == 0) {
			SendWeaponAnim(SOFLAM_SHOOT);
		}
		else {
			SendWeaponAnim(SOFLAM_SHOOT2);
		}

		// チェック
		bool ret = CheckAirStrike();

		// 成功している場合
		if (ret) {
			m_flNextPrimaryAttack = WeaponTimeBase() + SOFLAM_DELAY;
			m_flNextSecondaryAttack = WeaponTimeBase() + SOFLAM_DELAY;

			int randRadio = RANDOM_LONG(0, 2);
			const char* radioType;
			switch (randRadio) {
			case 1:  radioType = "hgrunt/yessir.wav";       break;
			case 2:  radioType = "hgrunt/affirmative.wav";  break;
			default: radioType = "hgrunt/roger.wav";        break;
			}
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, radioType, 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));

			m_lastRequested = WeaponTimeBase();

			m_strikeType = strikeType;
			m_strikeStatus = 0;
			m_lastStrike = WeaponTimeBase() + STRIKE_BEFORE;

			if (strikeType == 0) {
				UTIL_ClientPrint(m_pPlayer, print_center, "Fire support was requested!!\n\nTake cover!!");
			}
			else {
				UTIL_ClientPrint(m_pPlayer, print_center, "Wide range fire support was requested!!\n\nTake cover!!");
			}

			// 天井などで失敗している場合
		}
		else {
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "hgrunt/negative.wav", 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));

			m_flNextPrimaryAttack = WeaponTimeBase() + 1.0;
			m_flNextSecondaryAttack = WeaponTimeBase() + 1.0;

			UTIL_ClientPrint(m_pPlayer, print_center, "Can not execute fire support to there!!");
		}
	}

	bool CheckAirStrike() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return false;

		TraceResult tr;
		Vector vecSrc = m_pPlayer->GetGunPosition();

		// ■実行チェック
		MAKE_VECTORS(m_pPlayer->pev->v_angle);
		TRACE_LINE(vecSrc, vecSrc + gpGlobals->v_forward * 8192,
			ignore_monsters, m_pPlayer->edict(), &tr);

		// 注視点がSKYテクスチャなら、終了
		if (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY) {
			return false;

			// 注視点が壁
		}
		else {
			vecSrc = tr.vecEndPos;

			// ヒット地点上空がSKYではないなら、終了
			TRACE_LINE(vecSrc,
				vecSrc + Vector(0, 0, 180) * 8192, ignore_monsters, m_pPlayer->edict(), &tr);

			if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY) {
				return false;
			}

			m_strikeHitPos = vecSrc;
			m_strikeAbovePos = tr.vecEndPos;
		}

		return true;
	}

	void WeaponIdle() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		ResetEmptySound();

		bool isPointBonus = false;

		// レーザーポインタ（発射可能なら発光）
		if (((m_lastRequested == 0) || (WeaponTimeBase() > m_lastRequested + SOFLAM_DELAY)
			|| (m_strikeStatus != -1))
			) {
			TraceResult tr;
			Vector vecSrc = m_pPlayer->GetGunPosition();

			MAKE_VECTORS(m_pPlayer->pev->v_angle);
			TRACE_LINE(vecSrc, vecSrc + gpGlobals->v_forward * 8192,
				ignore_monsters, m_pPlayer->edict(), &tr);

			// 点
			uint8 alpha = 255;
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
			WRITE_BYTE(TE_GLOWSPRITE);
			WRITE_COORD(tr.vecEndPos.x);
			WRITE_COORD(tr.vecEndPos.y);
			WRITE_COORD(tr.vecEndPos.z);
			WRITE_SHORT(m_gDotSprite);
			WRITE_BYTE(1);  // life
			WRITE_BYTE(3); // scale
			WRITE_BYTE(alpha); // alpha
			MESSAGE_END();

			// レーザー
			uint8 r, g, b;

			r = 192;
			g = 0;
			b = 0;

			// 爆撃地点を照射中
			if ((m_strikeStatus != -1) && ((tr.vecEndPos - m_strikeHitPos).Length() < BONUS_RANGE)) {
				r = 0;
				g = 255;
				b = 255;
				isPointBonus = true;
			}

			vecSrc = vecSrc + gpGlobals->v_forward * 30 - gpGlobals->v_up * 6 + gpGlobals->v_right * 4;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(vecSrc.x);
			WRITE_COORD(vecSrc.y);
			WRITE_COORD(vecSrc.z);
			WRITE_COORD(tr.vecEndPos.x);
			WRITE_COORD(tr.vecEndPos.y);
			WRITE_COORD(tr.vecEndPos.z);
			WRITE_SHORT(m_gLaserSprite);
			WRITE_BYTE(0);   // frame start
			WRITE_BYTE(100); // frame end
			WRITE_BYTE(1);   // life
			WRITE_BYTE(4);  // width
			WRITE_BYTE(0);   // noise
			WRITE_BYTE(r);
			WRITE_BYTE(g);
			WRITE_BYTE(b);
			WRITE_BYTE(alpha);   // actually brightness
			WRITE_BYTE(32);  // scroll
			MESSAGE_END();

		}


		// ■爆撃処理
		if (m_strikeStatus >= STRIKE_CNT) {
			m_strikeStatus = -1;
		}
		if ((m_strikeStatus != -1) && (WeaponTimeBase() > m_lastStrike + STRIKE_INTERVAL)) {
			CBaseEntity* pRocket;

			if (m_strikeStatus == 0) {
				m_pPlayer->rgAmmo(m_iPrimaryAmmoType, m_pPlayer->rgAmmo(m_iPrimaryAmmoType) - 1);
			}

			int spreadMin = (m_strikeType == 0) ? -150 : -500;
			int spreadMax = (m_strikeType == 0) ? 150 : 500;

			Vector vecStrike = m_strikeAbovePos + Vector(0, 0, -20);

			// 爆撃地点照射中ならボーナス
			int count = (isPointBonus) ? 2 : 1;
			for (int i = 0; i < count; i++) {
				pRocket = ShootMortar(m_pPlayer->edict(),
					vecStrike + Vector(RANDOM_LONG(spreadMin, spreadMax), RANDOM_LONG(spreadMin, spreadMax), 0),
					Vector(0, 0, 0));

				pRocket->pev->velocity = Vector(
					RANDOM_LONG(-50, 50),
					RANDOM_LONG(-50, 50),
					RANDOM_LONG(-400, -250));
			}

			m_strikeStatus++;
			m_lastStrike = WeaponTimeBase();
		}


		if (m_flTimeWeaponIdle > WeaponTimeBase()) {
			return;
		}

		SendWeaponAnim(SOFLAM_IDLE1);

		m_flTimeWeaponIdle = WeaponTimeBase() + RANDOM_FLOAT(10, 15);// how long till we do this again.
	}
};

LINK_ENTITY_TO_CLASS(weapon_as_soflam, CSoflam);
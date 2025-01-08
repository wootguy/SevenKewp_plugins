#include "extdll.h"
#include "util.h"
#include "weaponbase.h"
#include "proj_bullet.h"

ItemInfo g_wepinfo_sc2sg = {
    2,								// iSlot
    1,								// iPosition (-1 = automatic)
    "buckshot",						// pszAmmo1
    100,				            // iMaxAmmo1
    NULL,         					// pszAmmo2
    6,				                // iMaxAmmo2
    "svencontra2/weapon_sc2sg",     // pszName (path to HUD config)
    -1,             				// iMaxClip
    -1,								// iId (-1 = automatic)
    CONTRA_WEP_FLAGS,              	// iFlags
    CONTRA_WEP_WEIGHT               // iWeight
};

class CWeaponSc2sg : public CBaseContraWeapon {
public:
    //霰弹圆形扩散度;
    float flRoundSpear = 50.0f;

    const char* GetDeathNoticeWeapon() override { return "weapon_shotgun"; }

    CWeaponSc2sg() {
        szVModel = "models/svencontra2/v_sc2sg_hl.mdl";
        szPModel = "models/svencontra2/wp_sc2sg.mdl";
        szWModel = "models/svencontra2/wp_sc2sg.mdl";
        szShellModel = "models/shotgunshell.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2sg.spr";

        wepinfo = &g_wepinfo_sc2sg;
        iDefaultAmmo = 80;

        flDeployTime = 0.8f;
        flPrimeFireTime = 0.8f;
        flSecconaryFireTime = 0.25f;

        szWeaponAnimeExt = "shotgun";

        iDeployAnime = 6;
        iReloadAnime = 3;
        aryFireAnime = { 1, 2 };
        aryIdleAnime = { 0, 8 };

        szFireSound = "weapons/svencontra2/shot_sg.wav";

        flBulletSpeed = 2000;
        flDamage = g_WeaponDMG.SG;
        vecPunchX = Vector2D(-1, 1);
        vecPunchY = Vector2D(-1, 1);
        iShellBounce = TE_BOUNCE_SHOTSHELL;
        vecEjectOffset = Vector(24, 8, -5);
    }
    void Precache() override {
        PRECACHE_SOUND("weapons/svencontra2/shot_sg.wav");

        PRECACHE_MODEL("sprites/svencontra2/hud_sc2sg.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_sg.spr");

        CBaseContraWeapon::Precache();
    }
    void SingleProj(float r, float u) {
        Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
        CProjBullet* pBullet = (CProjBullet*)CBaseEntity::Create(BULLET_REGISTERNAME, g_vecZero, g_vecZero, false);
        UTIL_SetOrigin(pBullet->pev, m_pPlayer->GetGunPosition());
        pBullet->pev->owner = m_pPlayer->edict();
        pBullet->pev->model = MAKE_STRING("sprites/svencontra2/bullet_sg.spr");
        pBullet->pev->velocity = vecAiming * flBulletSpeed + r * gpGlobals->v_right + u * gpGlobals->v_up;
        pBullet->pev->angles = UTIL_VecToAngles(pBullet->pev->velocity);
        pBullet->pev->dmg = flDamage;
        DispatchSpawn(pBullet->edict());
    }
    void CreateProj(int pellet = 1) override {
        SingleProj(0, 0);
        for (int i = 0; i < pellet - 1; i++) {
            float Angle = 2 * M_PI *float(i) / float(pellet - 1);
            SingleProj(flRoundSpear * cos(Angle), flRoundSpear * sin(Angle));
        }
    }
    void PrimaryAttack() override {
        if (m_pPlayer->rgAmmo(m_iPrimaryAmmoType) <= 0) {
            PlayEmptySound();
            m_flNextPrimaryAttack = WeaponTimeBase() + 0.15f;
            return;
        }
        Fire(10);
        if (m_pPlayer->rgAmmo(m_iPrimaryAmmoType) <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
        m_flNextPrimaryAttack = WeaponTimeBase() + flPrimeFireTime;
    }
    void SecondaryAttack() override {
        if (m_pPlayer->rgAmmo(m_iPrimaryAmmoType) <= 0) {
            PlayEmptySound();
            m_flNextSecondaryAttack = WeaponTimeBase() + 0.15f;
            return;
        }
        Fire(8);
        if (m_pPlayer->rgAmmo(m_iPrimaryAmmoType) <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
        m_flNextSecondaryAttack = WeaponTimeBase() + flSecconaryFireTime;
    }
};

LINK_ENTITY_TO_CLASS(weapon_sc2sg, CWeaponSc2sg)

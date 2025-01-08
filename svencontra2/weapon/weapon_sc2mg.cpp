#include "extdll.h"
#include "util.h"
#include "weaponbase.h"
#include "proj_bullet.h"

ItemInfo g_wepinfo_sc2mg = {
    2,								// iSlot
    -1,								// iPosition (-1 = automatic)
    "556",						    // pszAmmo1 (TODO: 556)
    200,				            // iMaxAmmo1 (reduced for HL clients -w00tguy)
    NULL,         					// pszAmmo2
    -1,				                // iMaxAmmo2
    "svencontra2/weapon_sc2mg",     // pszName (path to HUD config)
    -1,             				// iMaxClip
    -1,								// iId (-1 = automatic)
    CONTRA_WEP_FLAGS,              	// iFlags
    CONTRA_WEP_WEIGHT               // iWeight
};

class CWeaponSc2mg : public CBaseContraWeapon {
public:
    int iMaxBurstFire = 4;
    int iBurstLeft = 0;
    float flBurstTime = 0.035;
    float flNextBurstTime;

    const char* GetDeathNoticeWeapon() override { return "weapon_9mmAR"; }

    CWeaponSc2mg() {
        szVModel = "models/svencontra2/v_sc2mg_hl.mdl";
        szPModel = "models/svencontra2/wp_sc2mg.mdl";
        szWModel = "models/svencontra2/wp_sc2mg.mdl";
        szShellModel = "models/saw_shell.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2mg.spr";

        wepinfo = &g_wepinfo_sc2mg;
        //iDefaultAmmo = 300;
        iDefaultAmmo = 200; // reduced for HL clients -w00tguy

        flDeployTime = 0.8f;
        flPrimeFireTime = 0.09f;
        flSecconaryFireTime = 0.5f;

        //szWeaponAnimeExt = "m16";
        szWeaponAnimeExt = "mp5";

        iDeployAnime = 5;
        iReloadAnime = 3;
        aryFireAnime = { 6, 7, 8 };
        aryIdleAnime = { 0, 1 };

        szFireSound = "weapons/svencontra2/shot_mg.wav";

        flBulletSpeed = 2000;
        flDamage = g_WeaponDMG.MG;
        vecPunchX = Vector2D(-1, 1);
        vecPunchY = Vector2D(-1, 1);
        vecEjectOffset = Vector(24, 8, -5);
    }
    void Precache() override {
        PRECACHE_SOUND("weapons/svencontra2/shot_mg.wav");

        PRECACHE_MODEL("sprites/svencontra2/hud_sc2mg.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_mg.spr");

        CBaseContraWeapon::Precache();
    }
    void Holster(int skiplocal /* = 0 */) override {
        iBurstLeft = 0;
        flNextBurstTime = 0;
        SetThink(NULL);
        CBaseContraWeapon::Holster(skiplocal);
    }
    void CreateProj(int pellet = 1) override {
        CProjBullet* pBullet = (CProjBullet*)CBaseEntity::Create(BULLET_REGISTERNAME, g_vecZero, g_vecZero, false);
        UTIL_SetOrigin(pBullet->pev, m_pPlayer->GetGunPosition());
        pBullet->pev->owner = m_pPlayer->edict();
        pBullet->pev->model = MAKE_STRING("sprites/svencontra2/bullet_mg.spr");
        pBullet->pev->velocity = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES) * flBulletSpeed;
        pBullet->pev->angles = UTIL_VecToAngles(pBullet->pev->velocity);
        pBullet->pev->dmg = flDamage;
        DispatchSpawn(pBullet->edict());
    }
    void SecondaryAttack() override {
        CBaseContraWeapon::SecondaryAttack();
        iBurstLeft = iMaxBurstFire - 1;
        flNextBurstTime = WeaponTimeBase() + flBurstTime;
        m_flNextSecondaryAttack = m_flNextPrimaryAttack = WeaponTimeBase() + flSecconaryFireTime;
    }
    void ItemPostFrame() override {
        if (iBurstLeft > 0) {
            if (flNextBurstTime < WeaponTimeBase()) {
                if (m_pPlayer->rgAmmo(m_iPrimaryAmmoType) <= 0) {
                    iBurstLeft = 0;
                    return;
                }
                else
                    iBurstLeft--;
                Fire();

                if (iBurstLeft > 0)
                    flNextBurstTime = WeaponTimeBase() + flBurstTime;
                else
                    flNextBurstTime = 0;
            }
            return;
        }
        CBasePlayerWeapon::ItemPostFrame();
    }
};

LINK_ENTITY_TO_CLASS(weapon_sc2mg, CWeaponSc2mg)

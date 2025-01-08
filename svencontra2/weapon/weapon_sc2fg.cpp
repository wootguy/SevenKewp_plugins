#include "extdll.h"
#include "util.h"
#include "weaponbase.h"
#include "proj_bullet.h"

ItemInfo g_wepinfo_sc2fg = {
    3,								// iSlot
    4,								// iPosition (-1 = automatic)
    "rockets",						// pszAmmo1
    100,				            // iMaxAmmo1
    NULL,         					// pszAmmo2
    -1,				                // iMaxAmmo2
    "svencontra2/weapon_sc2fg",     // pszName (path to HUD config)
    -1,             				// iMaxClip
    -1,								// iId (-1 = automatic)
    CONTRA_WEP_FLAGS,              	// iFlags
    CONTRA_WEP_WEIGHT               // iWeight
};

class CWeaponSc2fg : public CBaseContraWeapon {
public:
    const char* GetDeathNoticeWeapon() override { return "rpg_rocket"; }

    CWeaponSc2fg() {
        szVModel = "models/svencontra2/v_sc2fg.mdl";
        szPModel = "models/svencontra2/wp_sc2fg.mdl";
        szWModel = "models/svencontra2/wp_sc2fg.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2fg.spr";

        wepinfo = &g_wepinfo_sc2fg;
        iDefaultAmmo = 40;

        flDeployTime = 0.8f;
        flPrimeFireTime = 0.6f;
        flSecconaryFireTime = 0.6f;

        //szWeaponAnimeExt = "m16";
        szWeaponAnimeExt = "mp5";

        iDeployAnime = 5;
        iReloadAnime = 3;
        aryFireAnime = { 1, 2 };
        aryIdleAnime = { 0 };

        szFireSound = "weapons/svencontra2/shot_fg.wav";

        flBulletSpeed = 2200;
        flDamage = g_WeaponDMG.FG;
        vecPunchX = Vector2D(-4, 5);
        vecPunchY = Vector2D(-1, 1);
        vecEjectOffset = Vector(0, 2, 0);
    }
    void Precache() override {
        PRECACHE_SOUND("weapons/svencontra2/shot_fg.wav");
        PRECACHE_SOUND("weapons/svencontra2/shot_fghit.wav");

        PRECACHE_MODEL("sprites/svencontra2/hud_sc2fg.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_fg.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_fghit.spr");

        CBaseContraWeapon::Precache();
    }
    void CreateProj(int pellet = 1) override {
        CProjBullet* pBullet = (CProjBullet*)(CBaseEntity::Create(BULLET_REGISTERNAME, g_vecZero, g_vecZero, false));
        UTIL_SetOrigin(pBullet->pev, m_pPlayer->GetGunPosition());
        pBullet->pev->owner = m_pPlayer->edict();
        pBullet->pev->dmg = flDamage;
        pBullet->pev->model = MAKE_STRING("sprites/svencontra2/bullet_fg.spr");
        //爆炸SPR, 爆炸音效, SPR缩放, 伤害范围, 伤害
        pBullet->SetExpVar("sprites/svencontra2/bullet_fghit.spr", "weapons/svencontra2/shot_fghit.wav", 10, 128, g_WeaponDMG.FGE);
        pBullet->pev->velocity = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES) * flBulletSpeed;
        pBullet->pev->angles = UTIL_VecToAngles(pBullet->pev->velocity);
        pBullet->pTouchFunc = ProjBulletTouch::ExplodeTouch;
        DispatchSpawn(pBullet->edict());
    }
};

LINK_ENTITY_TO_CLASS(weapon_sc2fg, CWeaponSc2fg)
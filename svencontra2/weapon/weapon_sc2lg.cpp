#include "extdll.h"
#include "util.h"
#include "weaponbase.h"
#include "proj_bullet.h"

namespace ProjBulletTouch{
    void LaserGunFirstShotTouch(CProjBullet* pThis, CBaseEntity* pOther){
        pThis->pev->velocity = g_vecZero;
    }
    void LaserGunShotTouch(CProjBullet* pThis, CBaseEntity* pOther){
        ProjBulletTouch::DefaultDirectTouch(pThis, pOther);
        if(pOther->IsBSPModel()){
            TraceResult tr;
            Vector vecStart = pThis->pev->origin;
            Vector vecEnd = vecStart + pThis->pev->velocity;
            UTIL_TraceHull(vecStart, vecEnd, missile, point_hull, pThis->edict(), &tr);
            DecalGunshot(&tr, BULLET_PLAYER_357);
            UTIL_Sparks(vecStart);
        }
        ProjBulletTouch::DefaultPostTouch(pThis, pOther);
    }
    void LaserGunLastShotTouch(CProjBullet* pThis, CBaseEntity* pOther){
        ProjBulletTouch::DefaultDirectTouch(pThis, pOther);
        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
            WRITE_BYTE(TE_KILLBEAM);
            WRITE_SHORT(pThis->entindex());
        MESSAGE_END();
        CBaseEntity* pFirst = CBaseEntity::Instance(pThis->pev->euser2);
        UTIL_Remove(pFirst);
        ProjBulletTouch::DefaultPostTouch(pThis, pOther);
    }
}

ItemInfo g_wepinfo_sc2lg = {
    3,								// iSlot
    3,								// iPosition (-1 = automatic)
    "uranium",						// pszAmmo1
    100,				            // iMaxAmmo1
    NULL,       					// pszAmmo2
    -1,				                // iMaxAmmo2
    "svencontra2/weapon_sc2lg",     // pszName (path to HUD config)
    -1,             				// iMaxClip
    -1,								// iId (-1 = automatic)
    CONTRA_WEP_FLAGS,              	// iFlags
    CONTRA_WEP_WEIGHT               // iWeight
};

class CWeaponSc2lg : public CBaseContraWeapon{
public:
    //激光宽度
    uint8 uiBeamWidth = 40;
    //激光SPR
    const char* szBeamSpr = "sprites/svencontra2/lgbeam.spr";
    //激光伤害间隔
    float flShotFireInterv = 0.01;
    //激光伤害总数
    int iShotMaxFire = 10;

    int iShotFire = iShotMaxFire;
    bool bInFiring = false;
    EHANDLE eFirst;
    Vector vecOldVel;

    const char* GetDeathNoticeWeapon() override { return "weapon_gauss"; }

    CWeaponSc2lg(){
        szVModel = "models/svencontra2/v_sc2lg.mdl";
        szPModel = "models/svencontra2/wp_sc2lg.mdl";
        szWModel = "models/svencontra2/wp_sc2lg.mdl";
        szShellModel = "models/saw_shell.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2lg.spr";

        wepinfo = &g_wepinfo_sc2lg;
        iDefaultAmmo = 50;

        flDeployTime = 0.8f;
        flPrimeFireTime = 1.0f;

        //szWeaponAnimeExt = "m16";
        szWeaponAnimeExt = "mp5";

        iDeployAnime = 8;
        iReloadAnime = 0;
        aryFireAnime = {5, 6};
        aryIdleAnime = {0, 1};

        szFireSound = "weapons/svencontra2/shot_lg.wav";

        flBulletSpeed = 4000;
        flDamage = g_WeaponDMG.LG;
        vecPunchX = Vector2D(-1,1);
        vecPunchY = Vector2D(-1,1);
        vecEjectOffset = Vector(24,8,-5);
    }
    void Precache() override{
        PRECACHE_SOUND( "weapons/svencontra2/shot_lg.wav" );

        PRECACHE_MODEL(szBeamSpr);
        PRECACHE_MODEL("sprites/svencontra2/hud_sc2lg.spr");

        CBaseContraWeapon::Precache();
    }
    BOOL CanHolster(){
        return !bInFiring;
    }
    void KillBeam(int entindex){
        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
            WRITE_BYTE(TE_KILLBEAM);
            WRITE_SHORT(entindex);
        MESSAGE_END();
    }
    void KillBeam(CBaseEntity* pWho){
        KillBeam(pWho->entindex());
    }
    void EntBeam(int eindex1, int eindex2){
        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
            WRITE_BYTE(TE_BEAMENTS);
            WRITE_SHORT(eindex1);
            WRITE_SHORT(eindex2);
            WRITE_SHORT(g_engfuncs.pfnModelIndex(szBeamSpr));
            WRITE_BYTE(0);
            WRITE_BYTE(0);
            WRITE_BYTE(255);
            WRITE_BYTE(uiBeamWidth);
            WRITE_BYTE(0);
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            WRITE_BYTE(255); // actually brightness
            WRITE_BYTE(0);
        MESSAGE_END();
    }
    void Holster(int skiplocal){
        KillBeam(m_pPlayer->entindex() + 4096);
        bInFiring = false;
        CBaseContraWeapon::Holster(skiplocal);
    }
    void PrimaryAttack() override{
        KillBeam(m_pPlayer->entindex() + 4096);
        CBaseContraWeapon::PrimaryAttack();
        m_flNextSecondaryAttack = WeaponTimeBase() + flPrimeFireTime;
    }
    void SecondaryAttack() override{
        PrimaryAttack();
    }
    CProjBullet* CreateIvisibleProj(){
        CProjBullet* pBullet = (CProjBullet*)CBaseEntity::Create( BULLET_REGISTERNAME, g_vecZero, g_vecZero, false);
        UTIL_SetOrigin( pBullet->pev, m_pPlayer->GetGunPosition() );
        pBullet->pev->owner = m_pPlayer->edict();
        pBullet->pev->dmg = flDamage;
        DispatchSpawn( pBullet->edict() );
        //不能用no_draw与model=0, 否则将不会被绘制
        pBullet->pev->rendermode = kRenderTransAdd;
        pBullet->pev->renderamt = 0;
        return pBullet;
    }
    void CreateProj(int pellet = 1) override{
        bInFiring = true;
        SetThink(&CWeaponSc2lg::ShotFireThink);
        pev->nextthink = WeaponTimeBase() + flShotFireInterv;
        CProjBullet* pBullet = CreateIvisibleProj();
        pBullet->pev->velocity = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES ) * flBulletSpeed;
        pBullet->pev->angles = UTIL_VecToAngles( pBullet->pev->velocity );
        EntBeam(pBullet->entindex(), m_pPlayer->entindex() + 4096);
        pBullet->pTouchFunc = ProjBulletTouch::LaserGunFirstShotTouch;
        vecOldVel = pBullet->pev->velocity;
        eFirst = EHANDLE(pBullet->edict());
    }
    void ShotFireThink(){
        CProjBullet* pBullet = CreateIvisibleProj();
        pBullet->pev->velocity = vecOldVel;
        if(iShotFire > 0){
            pBullet->pTouchFunc = ProjBulletTouch::LaserGunShotTouch;
            iShotFire--;
            pev->nextthink = WeaponTimeBase() + flShotFireInterv;
        } 
        else{
            if(eFirst){
                pBullet->pTouchFunc = ProjBulletTouch::LaserGunLastShotTouch;
                CBaseEntity* pEntity = eFirst.GetEntity();
                pBullet->pev->euser2 = pEntity->edict();
                KillBeam(pEntity->entindex());
                EntBeam(pEntity->entindex(), pBullet->entindex());
            }
            iShotFire = iShotMaxFire;
            bInFiring = false;
        }
    }
};

LINK_ENTITY_TO_CLASS(weapon_sc2lg, CWeaponSc2lg)

#include "extdll.h"
#include "util.h"
#include "weaponbase.h"
#include "dynamicdifficult.h"
#include "proj_bullet.h"
#include "CSoundEnt.h"
#include "CGrenade.h"

ItemInfo g_wepinfo_sc2ar = {
    1,								// iSlot
    4,								// iPosition (-1 = automatic)
    "9mm",						// pszAmmo1
    100,				            // iMaxAmmo1
    "ARgrenades",					// pszAmmo2
    6,				                // iMaxAmmo2
    "svencontra2/weapon_sc2ar",     // pszName (path to HUD config)
    -1,             				// iMaxClip
    -1,								// iId (-1 = automatic)
    CONTRA_WEP_FLAGS,	            // iFlags
    CONTRA_WEP_WEIGHT   			// iWeight
};

class CWeaponSc2ar : public CBaseContraWeapon {
public:
     bool bInRecharg = false;
     float flRechargInterv = 0.02;
     const char* szGrenadeSpr = "sprites/svencontra2/bullet_gr.spr";
     const char* szGrenadeFireSound = "weapons/svencontra2/shot_gr.wav";

     const char* GetDeathNoticeWeapon() override { return "weapon_9mmAR"; }

     CWeaponSc2ar() {
        szVModel = "models/svencontra2/v_sc2ar.mdl";
        szPModel = "models/svencontra2/wp_sc2ar.mdl";
        szWModel = "models/svencontra2/wp_sc2ar.mdl";
        szShellModel = "models/saw_shell.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2ar.spr";
        
        wepinfo = &g_wepinfo_sc2ar;
        iDefaultAmmo = 100;

        flDeployTime = 0.8f;
        flPrimeFireTime = 0.11f;
        flSecconaryFireTime = 1.5f;

        //szWeaponAnimeExt = "m16";
        szWeaponAnimeExt = "mp5";

        iDeployAnime = 4;
        iReloadAnime = 3;
        aryFireAnime = {5, 6, 7};
        aryIdleAnime = {0, 1};

        szFireSound = "weapons/svencontra2/shot_ar.wav";

        flBulletSpeed = 1900;
        flDamage = g_WeaponDMG.AR;
        vecPunchX = Vector2D(-1,1);
        vecPunchY = Vector2D(-1,1);
        vecEjectOffset = Vector(24,8,-5);
     }
     void Precache() override{
        PRECACHE_SOUND( "weapons/svencontra2/shot_ar.wav" );
        PRECACHE_SOUND( "weapons/svencontra2/shot_gr.wav" );
        PRECACHE_SOUND( szGrenadeFireSound );

        PRECACHE_MODEL("sprites/svencontra2/bullet_ar.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_gr.spr");
        PRECACHE_MODEL("sprites/svencontra2/hud_sc2ar.spr");
        PRECACHE_MODEL(szGrenadeSpr);

        CBaseContraWeapon::Precache();
     }
     void Holster(int skiplocal){
         bInRecharg = false;
         CBaseContraWeapon::Holster(skiplocal);
     }
     void CreateProj(int pellet = 1) override{
        CProjBullet* pBullet = (CProjBullet*)CBaseEntity::Create(BULLET_REGISTERNAME, g_vecZero, g_vecZero, false);
        UTIL_SetOrigin( pBullet->pev, m_pPlayer->GetGunPosition() );
        pBullet->pev->owner = m_pPlayer->edict();
        pBullet->pev->model = MAKE_STRING("sprites/svencontra2/bullet_ar.spr");
        pBullet->pev->velocity = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES ) * flBulletSpeed;
        pBullet->pev->angles = UTIL_VecToAngles( pBullet->pev->velocity );
        pBullet->pev->dmg = flDamage;
        DispatchSpawn( pBullet->edict() );
    }
    void RechargeThink(){
        if(m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) >= wepinfo->iMaxAmmo1){
            bInRecharg = false;
            SetThink(NULL);
            return;
        }
        PlayEmptySound();
        m_pPlayer->rgAmmo( m_iPrimaryAmmoType, m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) + 1 );
        pev->nextthink = WeaponTimeBase() + flRechargInterv;
    }
    void Recharge(){
        bInRecharg = true;
        SetThink(&CWeaponSc2ar::RechargeThink);
        pev->nextthink = WeaponTimeBase() + flRechargInterv;
    }
    void PrimaryAttack(){
        if(bInRecharg)
            return;
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0 && !bInRecharg){
            PlayEmptySound();
            Recharge();
            return;
        }
        Fire();
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0 )
            m_pPlayer->SetSuitUpdate( "!HEV_AMO0", false, 0 );
        m_flNextPrimaryAttack = WeaponTimeBase() + flPrimeFireTime;
    }
    void SecondaryAttack() override{    
        if(bInRecharg)
            return;
        if( m_pPlayer->rgAmmo( m_iSecondaryAmmoType ) <= 0){
            PlayEmptySound();
            return;
        }    
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
        m_pPlayer->m_flStopExtraSoundTime = WeaponTimeBase() + 0.2;
        m_pPlayer->rgAmmo( m_iSecondaryAmmoType, m_pPlayer->rgAmmo( m_iSecondaryAmmoType ) - 1 );
        m_pPlayer->pev->punchangle.x = -10.0;
        EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, szGrenadeFireSound, 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );
        SendWeaponAnim(GetRandomAnime(aryFireAnime));
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        MAKE_VECTORS( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
        CGrenade* pGrenade = CGrenade::ShootContact( m_pPlayer->pev, 
            m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 12 + gpGlobals->v_right * 6,  gpGlobals->v_forward * 800 );
        SET_MODEL(pGrenade->edict(), szGrenadeSpr);
        pGrenade->pev->rendermode = kRenderTransAdd;
        pGrenade->pev->renderamt = 255;
        pGrenade->pev->rendercolor = Vector(255, 255, 255);
        pGrenade->pev->avelocity = g_vecZero;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = WeaponTimeBase() + flSecconaryFireTime;
        m_flTimeWeaponIdle = WeaponTimeBase() + 5;
    }
};

LINK_ENTITY_TO_CLASS(weapon_sc2ar, CWeaponSc2ar)

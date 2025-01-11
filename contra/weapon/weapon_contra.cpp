#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "ammobase.h"
#include "proj_bullet.h"
#include "weapon_contra.h"

#define CONTRA_WEAPON_VMDL "models/v_9mmAR.mdl"
#define CONTRA_WEAPON_PMDL "models/p_9mmAR.mdl"
#define CONTRA_WEAPON_WMDL "models/w_9mmAR.mdl"
#define CONTRA_WEAPON_ANIM "mp5"
#define CONTRA_WEAPON_SHOOTSND "sc_contrahdl/hdl_shot.wav"
#define CONTRA_WEAPON_FLAG (ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_NOAUTORELOAD | ITEM_FLAG_NOAUTOSWITCHEMPTY)
#define CONTRA_WEAPON_REGEN_TIME 7.0f
#define CONTRA_WEAPON_SLOT 3
#define CONTRA_WEAPON_POSITION 4
#define CONTRA_WEAPON_SHELLMDL "models/shell.mdl"
#define CONTRA_WEAPON_MAXAMMO 1
#define CONTRA_WEAPON_RECHARGE_GAP 0.1

enum CONTRA_WEAPON_ANIMATION
{
	CONTRA_WEAPON_LONGIDLE = 0,
	CONTRA_WEAPON_IDLE1,
	CONTRA_WEAPON_LAUNCH,//HelloTimber added. Wanna fix the problem that no sound when fire.
	CONTRA_WEAPON_RELOAD,
	CONTRA_WEAPON_DEPLOY,
	CONTRA_WEAPON_FIRE1,
	CONTRA_WEAPON_FIRE2,//HelloTimber added. Wanna fix the wrong animation when fire.
	CONTRA_WEAPON_FIRE3,//HelloTimber added. Wanna fix the wrong animation when fire.
};

ItemInfo g_contra_wep_info = {
	CONTRA_WEAPON_SLOT,				// iSlot
	CONTRA_WEAPON_POSITION,			// iPosition (-1 = automatic)
	NULL,							// pszAmmo1
	CONTRA_WEAPON_MAXAMMO,			// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"weapon_contra",				// pszName (path to HUD config)
	CONTRA_WEAPON_MAXAMMO,			// iMaxClip
	-1,								// iId (-1 = automatic)
	CONTRA_WEAPON_FLAG,				// iFlags
	998								// iWeight
};

void CContraWeapon::Spawn()
{
	m_iAmmoType = &aryAmmoType[0];
	Precache();
	SET_MODEL( edict(), CONTRA_WEAPON_WMDL);
	m_iDefaultAmmo = CONTRA_WEAPON_MAXAMMO;
	FallInit();
}

void CContraWeapon::Precache()
{
	m_defaultModelV = CONTRA_WEAPON_VMDL;
	m_defaultModelP = CONTRA_WEAPON_PMDL;
	m_defaultModelW = CONTRA_WEAPON_WMDL;
	CBasePlayerWeapon::Precache();
	m_iShell = PRECACHE_MODEL( CONTRA_WEAPON_SHELLMDL );

	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( CONTRA_WEAPON_SHOOTSND );

	UTIL_PrecacheOther(BULLET_REGISTERNAME);
	UTIL_PrecacheOther("NAmmo");
	UTIL_PrecacheOther("MAmmo");
	UTIL_PrecacheOther("SAmmo");
	UTIL_PrecacheOther("LAmmo");
}

BOOL CContraWeapon::GetItemInfo( ItemInfo* info )
{
	*info = g_contra_wep_info;
	return TRUE;
}

BOOL CContraWeapon::AddToPlayer(CBasePlayer* pPlayer)
{
	if( !CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return false;
			
	m_pPlayer = pPlayer;
	return true;
}

void CContraWeapon::Holster( int skiplocal /* = 0 */ )
{
	bInReload = false;		
	SetThink( NULL );
}
	
BOOL CContraWeapon::PlayEmptySound()
{
	if( m_iPlayEmptySound )
	{
		m_iPlayEmptySound = FALSE;
		EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "hl/weapons/357_cock1.wav", 0.8, ATTN_NORM, 0, PITCH_NORM );
	}
	return false;
}

BOOL CContraWeapon::Deploy()
{
	return DefaultDeploy( GetModelV(), GetModelP(), CONTRA_WEAPON_DEPLOY, CONTRA_WEAPON_ANIM );
}
	
float CContraWeapon::WeaponTimeBase()
{
	return gpGlobals->time;
}

void CContraWeapon::CancelReload()
{
	SetThink( NULL );
	bInReload = false;
}

void CContraWeapon::PrimaryAttack()
{
	/*
	if( m_iClip <= 0 || bInReload)
	{
		PlayEmptySound();
		return;
	}
	*/
		
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;//HelloTimber added. Wanna fix the problem that no sound when fire.
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;//HelloTimber added. Wanna fix the problem that no sound when fire.

	//m_iClip--;
		
	EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, CONTRA_WEAPON_SHOOTSND, 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );//HelloTimber added. Wanna fix the problem that no sound when fire.

	Vector vecSrc = m_pPlayer->GetGunPosition();
	m_iAmmoType->Method( m_pPlayer, vecSrc, gpGlobals->v_forward * BULLET_DEFAULT_SPEED );
	SendWeaponAnim( CONTRA_WEAPON_FIRE1 );
    m_flNextPrimaryAttack = WeaponTimeBase() + m_iAmmoType->FireInter;
		
	Reload();//HelloTimber added. Needn't to reload manually.
}

void CContraWeapon::RechargeThink()
{
	if(m_iClip >= iMaxClip())
	{
		CancelReload();
		return;
	}
	m_iClip++;
	pev->nextthink = WeaponTimeBase() + CONTRA_WEAPON_RECHARGE_GAP;
}

void CContraWeapon::Reload()
{
	if(bInReload)
		return;

	m_flRechargeTime = WeaponTimeBase() + CONTRA_WEAPON_REGEN_TIME;
	bInReload = true;
	SetThink( &CContraWeapon::RechargeThink );
	pev->nextthink = WeaponTimeBase();
	CBasePlayerWeapon::Reload();
}

void CContraWeapon::WeaponIdle()
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if( m_flTimeWeaponIdle > WeaponTimeBase() )
		return;

	SendWeaponAnim( RANDOM_LONG( 0, 1 ) == 0 ? CONTRA_WEAPON_LONGIDLE : CONTRA_WEAPON_IDLE1 );
	m_flTimeWeaponIdle = WeaponTimeBase() + RANDOM_FLOAT( 10, 15 );
}

LINK_ENTITY_TO_CLASS(weapon_contra, CContraWeapon)

string CONTRA_WEAPON_VMDL = "models/hl/v_9mmAR.mdl";
string CONTRA_WEAPON_PMDL = "models/hl/p_9mmAR.mdl";
string CONTRA_WEAPON_WMDL = "models/hl/w_9mmAR.mdl";
string CONTRA_WEAPON_ANIM = "mp5";
string CONTRA_WEAPON_SHOOTSND = "hl/weapons/357_cock1.wav";
int CONTRA_WEAPON_FLAG = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_NOAUTORELOAD | ITEM_FLAG_NOAUTOSWITCHEMPTY;
float CONTRA_WEAPON_REGEN_TIME = 7.0f;
int CONTRA_WEAPON_SLOT = 3;
int CONTRA_WEAPON_POSITION = 4;
string CONTRA_WEAPON_SHELLMDL = "models/shell.mdl";
int CONTRA_WEAPON_MAXAMMO = 1;
float CONTRA_WEAPON_RECHARGE_GAP = 0.1;

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

class CContraWeapon : public CBasePlayerWeapon
{
	private CBasePlayer* m_pPlayer = NULL;
	private int m_iShell;
	private float m_flRechargeTime;
	private bool bInReload = false;
	CAmmoType* m_iAmmoType = aryAmmoType[0];

	void Spawn()
	{
		Precache();
		SET_MODEL( self, CONTRA_WEAPON_WMDL );
		m_iDefaultAmmo = CONTRA_WEAPON_MAXAMMO;
		FallInit();
	}

	void Precache()
	{
		PrecacheCustomModels();
		PRECACHE_MODEL( CONTRA_WEAPON_VMDL );
		PRECACHE_MODEL( CONTRA_WEAPON_PMDL );
		PRECACHE_MODEL( CONTRA_WEAPON_WMDL );
		m_iShell = PRECACHE_MODEL( CONTRA_WEAPON_SHELLMDL );

		PRECACHE_SOUND( "hl/weapons/357_cock1.wav" );
		PRECACHE_SOUND( CONTRA_WEAPON_SHOOTSND );

		g_Game.PrecacheGeneric( "sound/" + "hl/weapons/357_cock1.wav" );
		g_Game.PrecacheGeneric( "sound/" + CONTRA_WEAPON_SHOOTSND );
		g_Game.PrecacheGeneric( CONTRA_WEAPON_VMDL );
		g_Game.PrecacheGeneric( CONTRA_WEAPON_PMDL );
		g_Game.PrecacheGeneric( CONTRA_WEAPON_WMDL );
		g_Game.PrecacheGeneric( CONTRA_WEAPON_SHELLMDL );

        g_Game.PrecacheOther(BULLET_REGISTERNAME);
		g_Game.PrecacheOther("NAmmo");
    	g_Game.PrecacheOther("MAmmo");
		g_Game.PrecacheOther("SAmmo");
		g_Game.PrecacheOther("LAmmo");
	}

	bool GetItemInfo( ItemInfo&info )
	{
		info.iMaxAmmo1 	= CONTRA_WEAPON_MAXAMMO;
		info.iMaxAmmo2 	= -1;
		info.iMaxClip 	= CONTRA_WEAPON_MAXAMMO;
		info.iSlot 		= CONTRA_WEAPON_SLOT;
		info.iPosition 	= CONTRA_WEAPON_POSITION;
		info.iFlags 	= CONTRA_WEAPON_FLAG;
		info.iWeight 	= 998;

		return true;
	}

	bool AddToPlayer( CBasePlayer* pPlayer )
	{
		if( !BaseClass.AddToPlayer( pPlayer ) )
			return false;
			
		*m_pPlayer = pPlayer;
			
		MESSAGE_BEGIN message( MSG_ONE, gmsgWeapPickup, pPlayer->edict() );
			message.WRITE_LONG( m_iId );
		message.MESSAGE_END();
		return true;
	}

	void Holster( int skiplocal /* = 0 */ )
	{
		bInReload = false;		
		SetThink( null );
	}
	
	bool PlayEmptySound()
	{
		if( m_bPlayEmptySound )
		{
			m_bPlayEmptySound = false;
			EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "hl/weapons/357_cock1.wav", 0.8, ATTN_NORM, 0, PITCH_NORM );
		}
		return false;
	}

	bool Deploy()
	{
		return DefaultDeploy( GetV_Model( CONTRA_WEAPON_VMDL ), GetP_Model( CONTRA_WEAPON_PMDL ), CONTRA_WEAPON_DEPLOY, CONTRA_WEAPON_ANIM );
	}
	
	float WeaponTimeBase()
	{
		return gpGlobals->time;
	}

	void CancelReload()
	{
		SetThink( null );
		bInReload = false;
	}

	void PrimaryAttack()
	{
		if( m_iClip <= 0 || bInReload)
		{
			PlayEmptySound();
			return;
		}
		
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;//HelloTimber added. Wanna fix the problem that no sound when fire.
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;//HelloTimber added. Wanna fix the problem that no sound when fire.

		m_iClip--;
		
		EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "sc_contrahdl/hdl_shot.wav", 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );//HelloTimber added. Wanna fix the problem that no sound when fire.

		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_iAmmoType.Method( m_pPlayer, vecSrc, gpGlobals->v_forward * BULLET_DEFAULT_SPEED );
		SendWeaponAnim( CONTRA_WEAPON_FIRE1 );
        m_flNextPrimaryAttack = WeaponTimeBase() + m_iAmmoType.FireInter;
		
		Reload();//HelloTimber added. Needn't to reload manually.
	}

	void RechargeThink()
	{
		if(m_iClip >= iMaxClip())
		{
			CancelReload();
			return;
		}
		m_iClip++;
		pev->nextthink = WeaponTimeBase() + CONTRA_WEAPON_RECHARGE_GAP;
	}

	void Reload()
	{
		if(bInReload)
			return;

		m_flRechargeTime = WeaponTimeBase() + CONTRA_WEAPON_REGEN_TIME;
		bInReload = true;
		SetThink( ThinkFunction( RechargeThink ) );
		pev->nextthink = WeaponTimeBase();
		BaseClass.Reload();
	}

	void WeaponIdle()
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

		if( m_flTimeWeaponIdle > WeaponTimeBase() )
			return;

		SendWeaponAnim( RANDOM_LONG( m_pPlayer->random_seed,  0, 1 ) == 0 ? CONTRA_WEAPON_LONGIDLE : CONTRA_WEAPON_IDLE1 );
		m_flTimeWeaponIdle = WeaponTimeBase() + RANDOM_FLOAT( m_pPlayer->random_seed,  10, 15 );
	}
}

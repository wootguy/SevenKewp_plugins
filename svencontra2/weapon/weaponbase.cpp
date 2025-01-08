abstract class CBaseContraWeapon : public CBasePlayerWeapon{
    protected CBasePlayer* m_pPlayer = NULL;
    protected int iShell = -1;
    
    protected string szWModel;
    protected string szPModel;
    protected string szVModel;
    protected string szShellModel;
    protected string szPickUpSound = "svencontra2/picked.wav";
    protected string szFloatFlagModel;

    protected int iMaxAmmo;
    protected int iMaxAmmo2 = -1;
    protected int iDefaultAmmo;
    protected int iSlot;
    protected int iPosition;

    protected float flDeployTime;
    protected float flPrimeFireTime;
    protected float flSecconaryFireTime;

    protected string szWeaponAnimeExt;
    protected int iDeployAnime;
    protected int iReloadAnime;
    protected vector<int> aryFireAnime;
    protected vector<int> aryIdleAnime;

    protected string szFireSound;

    protected Vector2D vecPunchX;
    protected Vector2D vecPunchY;
    protected float flBulletSpeed;
    protected float flDamage;
    protected TE_BOUNCE iShellBounce = TE_BOUNCE_SHELL;

    protected Vector vecEjectOffset;

    protected EHANDLEpFlagEntity = NULL;
    protected float flFlagHeight = 24;

    void Spawn(){
        Precache();
        SET_MODEL( self, szWModel );
        m_iDefaultAmmo = iDefaultAmmo;
        FallInit();
    }
    void Precache(){
        PRECACHE_SOUND( "weapons/svencontra2/ar_reload.wav" );
        g_Game.PrecacheGeneric( "sound/weapons/svencontra2/ar_reload.wav" );
        PRECACHE_SOUND( szPickUpSound );
        g_Game.PrecacheGeneric( "sound/" + szPickUpSound );
        PRECACHE_SOUND( szFireSound );
        g_Game.PrecacheGeneric( "sound/" + szFireSound );

        PRECACHE_MODEL( szWModel );
        PRECACHE_MODEL( szPModel );
        PRECACHE_MODEL( szVModel );
        if(!szShellModel.IsEmpty()){
            iShell = PRECACHE_MODEL( szShellModel );
            g_Game.PrecacheGeneric( szShellModel );
        }
        if(!szFloatFlagModel.IsEmpty()){
            PRECACHE_MODEL( szFloatFlagModel );
            g_Game.PrecacheGeneric( szFloatFlagModel );
        }
        g_Game.PrecacheGeneric( szWModel );
        g_Game.PrecacheGeneric( szPModel );
        g_Game.PrecacheGeneric( szVModel );
    }
    bool GetItemInfo( ItemInfo&info ){
        info.iMaxAmmo1     = iMaxAmmo;
        info.iMaxAmmo2     = iMaxAmmo2;
        info.iMaxClip     = -1;
        info.iSlot         = iSlot;
        info.iPosition     = iPosition;
        info.iFlags     = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_NOAUTOSWITCHEMPTY;
        info.iWeight     = 998;

        return true;
    }
    bool AddToPlayer( CBasePlayer* pPlayer ){
        if( !BaseClass.AddToPlayer( pPlayer ) )
            return false;
        *m_pPlayer = pPlayer;    
        MESSAGE_BEGIN message( MSG_ONE, gmsgWeapPickup, pPlayer->edict() );
            message.WRITE_LONG( m_iId );
        message.MESSAGE_END();
        EMIT_SOUND_DYN( pPlayer->edict(), CHAN_WEAPON, szPickUpSound, 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );
        if(pFlagEntity.IsValid())
            UTIL_Remove(pFlagEntity);
        return true;
    }
    void Materialize(){
        if(!szFloatFlagModel.IsEmpty() && !pFlagEntity.IsValid()){
            Vector vecOrigin = pev->origin;
            vecOrigin.z += flFlagHeight;
            CBaseEntity* pEntity = g_EntityFuncs.Create(WEAPONFLAG_REGISTERNAME, vecOrigin, pev->angles, true, edict());
            pEntity->pev->fov = flFlagHeight;
            SET_MODEL(*pEntity, szFloatFlagModel);
            g_EntityFuncs.DispatchSpawn( pEntity->edict() );
            pFlagEntity = EHandle(pEntity);
        }
        BaseClass.Materialize();
    }
    void UpdateOnRemove(){
        if(pFlagEntity.IsValid())
            UTIL_Remove(pFlagEntity);
    }
    void Holster( int skiplocal /* = 0 */ ){    
        SetThink( null );
        BaseClass.Holster();
    }
    bool PlayEmptySound(){
        if( m_bPlayEmptySound ){
            m_bPlayEmptySound = false;
            EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "weapons/svencontra2/ar_reload.wav", 0.8, ATTN_NORM, 0, PITCH_NORM );
        }
        return false;
    }
    bool Deploy(){
        bool bResult = true;
        bResult = DefaultDeploy( GetV_Model( szVModel ), GetP_Model( szPModel ), iDeployAnime, szWeaponAnimeExt );
        m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flDeployTime;
        return bResult;
    }
    int GetRandomAnime(vector<int>&in ary){
        return ary[RANDOM_LONG(m_pPlayer->random_seed,0,ary.length()-1)];
    }
    void CreateProj(int pellet = 1){
        //Dummy
    }
    void Fire(int pellet = 1){
        CreateProj(pellet);
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
        m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        m_pPlayer->rgAmmo(m_iPrimaryAmmoType, m_pPlayer->rgAmmo(m_iPrimaryAmmoType)-1);
        EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, szFireSound, 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );
        SendWeaponAnim(GetRandomAnime(aryFireAnime));
        m_pPlayer->pev->punchangle.x = RANDOM_FLOAT( vecPunchX.x, vecPunchX.y );
        m_pPlayer->pev->punchangle.y = RANDOM_FLOAT( vecPunchY.x, vecPunchY.y );
        if(iShell > -1)
            EjectBrass( 
                m_pPlayer->GetGunPosition() + gpGlobals->v_forward * vecEjectOffset.x + gpGlobals->v_right * vecEjectOffset.y + gpGlobals->v_up * vecEjectOffset.z, 
                m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_LONG(80,120),
                m_pPlayer->pev->angles[1], iShell, iShellBounce );
    }
    void PrimaryAttack(){
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0){
            PlayEmptySound();
            m_flNextPrimaryAttack = WeaponTimeBase() + 0.15f;
            return;
        }
        Fire();
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0 )
            m_pPlayer->SetSuitUpdate( "!HEV_AMO0", false, 0 );
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = WeaponTimeBase() + flPrimeFireTime;
    }
    void SecondaryAttack(){
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0){
            PlayEmptySound();
            m_flNextSecondaryAttack = WeaponTimeBase() + 0.15f;
            return;
        }
        Fire();
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0 )
            m_pPlayer->SetSuitUpdate( "!HEV_AMO0", false, 0 );
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = WeaponTimeBase() + flSecconaryFireTime;
    }
    void WeaponIdle(){
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
        if( m_flTimeWeaponIdle > WeaponTimeBase() )
            return;
        SendWeaponAnim(GetRandomAnime(aryIdleAnime));
        m_flTimeWeaponIdle = WeaponTimeBase() + RANDOM_FLOAT( m_pPlayer->random_seed,  10, 15 );
    }
    float WeaponTimeBase(){
        return gpGlobals->time;
    }
}
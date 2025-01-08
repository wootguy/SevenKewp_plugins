 class  weapon_sc2ar : CBaseContraWeapon{
     private bool bInRecharg = false;
     private float flRechargInterv = 0.02;
     private string szGrenadeSpr = "sprites/svencontra2/bullet_gr.spr";
     private string szGrenadeFireSound = "weapons/svencontra2/shot_gr.wav";
     weapon_sc2ar(){
        szVModel = "models/svencontra2/v_sc2ar.mdl";
        szPModel = "models/svencontra2/wp_sc2ar.mdl";
        szWModel = "models/svencontra2/wp_sc2ar.mdl";
        szShellModel = "models/saw_shell.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2ar.spr";
        iMaxAmmo = 100;
        iMaxAmmo2 = 6;
        iDefaultAmmo = 100;
        iSlot = 1;
        iPosition = 20;

        flDeployTime = 0.8f;
        flPrimeFireTime = 0.11f;
        flSecconaryFireTime = 1.5f;

        szWeaponAnimeExt = "m16";

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
        g_Game.PrecacheGeneric( "sound/" + szGrenadeFireSound );
        g_Game.PrecacheGeneric( "sound/weapons/svencontra2/shot_ar.wav" );
        g_Game.PrecacheGeneric( "sound/weapons/svencontra2/shot_gr.wav" );

        PRECACHE_MODEL("sprites/svencontra2/bullet_ar.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_gr.spr");
        PRECACHE_MODEL("sprites/svencontra2/hud_sc2ar.spr");
        PRECACHE_MODEL(szGrenadeSpr);
        g_Game.PrecacheGeneric( szGrenadeSpr );
        g_Game.PrecacheGeneric( "sprites/svencontra2/bullet_ar.spr" );
        g_Game.PrecacheGeneric( "sprites/svencontra2/bullet_gr.spr" );
        g_Game.PrecacheGeneric( "sprites/svencontra2/hud_sc2ar.spr" );

        g_Game.PrecacheGeneric( "sprites/svencontra2/weapon_sc2ar.txt" );

        CBaseContraWeapon::Precache();
     }
     void Holster(int skiplocal){
         bInRecharg = false;
         CBaseContraWeapon::Holster(skiplocal);
     }
     void CreateProj(int pellet = 1) override{
        CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));
        g_EntityFuncs.SetOrigin( pBullet.self, m_pPlayer->GetGunPosition() );
        *pBullet->pev->owner = *m_pPlayer->edict();
        pBullet->pev->model = "sprites/svencontra2/bullet_ar.spr";
        pBullet->pev->velocity = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES ) * flBulletSpeed;
        pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
        pBullet->pev->dmg = flDamage;
        g_EntityFuncs.DispatchSpawn( pBullet.edict() );
    }
    void RechargeThink(){
        if(m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) >= iMaxAmmo){
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
        SetThink(ThinkFunction(RechargeThink));
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
        CGrenade* pGrenade = g_EntityFuncs.ShootContact( m_pPlayer->pev, 
            m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 12 + gpGlobals->v_right * 6,  gpGlobals->v_forward * 800 );
        SET_MODEL(*pGrenade, szGrenadeSpr);
        pGrenade->pev->rendermode = kRenderTransAdd;
        pGrenade->pev->renderamt = 255;
        pGrenade->pev->rendercolor = Vector(255, 255, 255);
        pGrenade->pev->avelocity = g_vecZero;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = WeaponTimeBase() + flSecconaryFireTime;
        m_flTimeWeaponIdle = WeaponTimeBase() + 5;
    }
}

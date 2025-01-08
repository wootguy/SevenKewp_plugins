class  weapon_sc2sg : CBaseContraWeapon{
    //霰弹圆形扩散度;
    private float flRoundSpear = 50.0f;
     weapon_sc2sg(){
        szVModel = "models/svencontra2/v_sc2sg.mdl";
        szPModel = "models/svencontra2/wp_sc2sg.mdl";
        szWModel = "models/svencontra2/wp_sc2sg.mdl";
        szShellModel = "models/shotgunshell.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2sg.spr";
        
        iMaxAmmo = 200;
        iDefaultAmmo = 80;
        iSlot = 2;
        iPosition = 21;

        flDeployTime = 0.8f;
        flPrimeFireTime = 0.8f;
        flSecconaryFireTime = 0.25f;

        szWeaponAnimeExt = "shotgun";

        iDeployAnime = 6;
        iReloadAnime = 3;
        aryFireAnime = {1, 2};
        aryIdleAnime = {0, 8};

        szFireSound = "weapons/svencontra2/shot_sg.wav";

        flBulletSpeed = 2000;
        flDamage = g_WeaponDMG.SG;
        vecPunchX = Vector2D(-1,1);
        vecPunchY = Vector2D(-1,1);
        iShellBounce = TE_BOUNCE_SHOTSHELL;
        vecEjectOffset = Vector(24,8,-5);
     }
     void Precache() override{
        PRECACHE_SOUND( "weapons/svencontra2/shot_sg.wav" );
        g_Game.PrecacheGeneric( "sound/weapons/svencontra2/shot_sg.wav" );

        PRECACHE_MODEL("sprites/svencontra2/hud_sc2sg.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_sg.spr");
        g_Game.PrecacheGeneric("sprites/svencontra2/hud_sc2sg.spr");    
        g_Game.PrecacheGeneric("sprites/svencontra2/bullet_sg.spr");            

        g_Game.PrecacheGeneric( "sprites/svencontra2/weapon_sc2sg.txt" );

        CBaseContraWeapon::Precache();
     }
     void SingleProj(float r, float u){
         Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
         CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));
            g_EntityFuncs.SetOrigin( pBullet.self, m_pPlayer->GetGunPosition() );
            *pBullet->pev->owner = *m_pPlayer->edict();
            pBullet->pev->model = "sprites/svencontra2/bullet_sg.spr";
            pBullet->pev->velocity = vecAiming * flBulletSpeed + r * gpGlobals->v_right + u *  gpGlobals->v_up;
            pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
            pBullet->pev->dmg = flDamage;
            g_EntityFuncs.DispatchSpawn(pBullet.edict());
     }
     void CreateProj(int pellet = 1) override{
        SingleProj(0, 0);
        for(int i = 0; i < pellet - 1; i++){
            float Angle = 2 * Math.PI * float(i) / float(pellet-1);
            SingleProj(flRoundSpear * cos(Angle), flRoundSpear * sin(Angle));
        }
    }
    void PrimaryAttack() override{
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0){
            PlayEmptySound();
            m_flNextPrimaryAttack = WeaponTimeBase() + 0.15f;
            return;
        }
        Fire(10);
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0 )
            m_pPlayer->SetSuitUpdate( "!HEV_AMO0", false, 0 );
        m_flNextPrimaryAttack = WeaponTimeBase() + flPrimeFireTime;
    }
    void SecondaryAttack() override{
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0){
            PlayEmptySound();
            m_flNextSecondaryAttack = WeaponTimeBase() + 0.15f;
            return;
        }
        Fire(8);
        if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0 )
            m_pPlayer->SetSuitUpdate( "!HEV_AMO0", false, 0 );
        m_flNextSecondaryAttack = WeaponTimeBase() + flSecconaryFireTime;
    }
}
        

class  weapon_sc2mg : CBaseContraWeapon{
    private int iMaxBurstFire = 4;
    private int iBurstLeft = 0;
    private float flBurstTime = 0.035;
    private float flNextBurstTime;
     weapon_sc2mg(){
        szVModel = "models/svencontra2/v_sc2mg.mdl";
        szPModel = "models/svencontra2/wp_sc2mg.mdl";
        szWModel = "models/svencontra2/wp_sc2mg.mdl";
        szShellModel = "models/saw_shell.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2mg.spr";

        iMaxAmmo = 600;
        iDefaultAmmo = 300;
        iSlot = 2;
        iPosition = 20;

        flDeployTime = 0.8f;
        flPrimeFireTime = 0.09f;
        flSecconaryFireTime = 0.5f;

        szWeaponAnimeExt = "m16";

        iDeployAnime = 5;
        iReloadAnime = 3;
        aryFireAnime = {6, 7, 8};
        aryIdleAnime = {0, 1};

        szFireSound = "weapons/svencontra2/shot_mg.wav";

        flBulletSpeed = 2000;
        flDamage = g_WeaponDMG.MG;
        vecPunchX = Vector2D(-1,1);
        vecPunchY = Vector2D(-1,1);
        vecEjectOffset = Vector(24,8,-5);
     }
     void Precache() override{
        PRECACHE_SOUND( "weapons/svencontra2/shot_mg.wav" );
        g_Game.PrecacheGeneric( "sound/weapons/svencontra2/shot_mg.wav" );

        PRECACHE_MODEL("sprites/svencontra2/hud_sc2mg.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_mg.spr");
        g_Game.PrecacheGeneric("sprites/svencontra2/hud_sc2mg.spr");
        g_Game.PrecacheGeneric("sprites/svencontra2/bullet_mg.spr");    

        g_Game.PrecacheGeneric( "sprites/svencontra2/weapon_sc2mg.txt" );

        CBaseContraWeapon::Precache();
     }
     void Holster( int skiplocal /* = 0 */ ) override{
        iBurstLeft = 0;
        flNextBurstTime = 0;
        SetThink( null );
        CBaseContraWeapon::Holster(skiplocal);
    }
     void CreateProj(int pellet = 1) override{
        CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));
        g_EntityFuncs.SetOrigin( pBullet.self, m_pPlayer->GetGunPosition() );
        *pBullet->pev->owner = *m_pPlayer->edict();
        pBullet->pev->model = "sprites/svencontra2/bullet_mg.spr";
        pBullet->pev->velocity = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES ) * flBulletSpeed;
        pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
        pBullet->pev->dmg = flDamage;
        g_EntityFuncs.DispatchSpawn( pBullet.edict() );
    }
    void SecondaryAttack() override{
        CBaseContraWeapon::SecondaryAttack();
        iBurstLeft = iMaxBurstFire - 1;
        flNextBurstTime = WeaponTimeBase() + flBurstTime;
        m_flNextSecondaryAttack = m_flNextPrimaryAttack = WeaponTimeBase() + flSecconaryFireTime;
    }
    void ItemPostFrame(){
        if( iBurstLeft > 0 ){
            if( flNextBurstTime < WeaponTimeBase() ){
                if(m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0){
                    iBurstLeft = 0;
                    return;
                }
                else
                    iBurstLeft--;
                Fire();

                if( iBurstLeft > 0 )
                    flNextBurstTime = WeaponTimeBase() + flBurstTime;
                else
                    flNextBurstTime = 0;
            }
            return;
        }
        BaseClass.ItemPostFrame();
    }
}

 class  weapon_sc2fg : CBaseContraWeapon{
     weapon_sc2fg(){
       szVModel = "models/svencontra2/v_sc2fg.mdl";
        szPModel = "models/svencontra2/wp_sc2fg.mdl";
        szWModel = "models/svencontra2/wp_sc2fg.mdl";
        szFloatFlagModel = "sprites/svencontra2/icon_sc2fg.spr";

        szShellModel = "";

        iMaxAmmo = 100;
        iMaxAmmo2 = -1;
        iDefaultAmmo = 40;
        iSlot = 3;
        iPosition = 20;

        flDeployTime = 0.8f;
        flPrimeFireTime = 0.6f;
        flSecconaryFireTime = 0.6f;

        szWeaponAnimeExt = "m16";

        iDeployAnime = 5;
        iReloadAnime = 3;
        aryFireAnime = {1, 2};
        aryIdleAnime = {0};

        szFireSound = "weapons/svencontra2/shot_fg.wav";

        flBulletSpeed = 2200;
        flDamage = g_WeaponDMG.FG;
        vecPunchX = Vector2D(-4,5);
        vecPunchY = Vector2D(-1,1);
        vecEjectOffset = Vector(0,2,0);
     }
     void Precache() override{
        PRECACHE_SOUND( "weapons/svencontra2/shot_fg.wav" );
        PRECACHE_SOUND( "weapons/svencontra2/shot_fghit.wav" );
        g_Game.PrecacheGeneric( "sound/weapons/svencontra2/shot_fg.wav" );
        g_Game.PrecacheGeneric( "sound/weapons/svencontra2/shot_fghit.wav" );
        
        PRECACHE_MODEL("sprites/svencontra2/hud_sc2fg.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_fg.spr");
        PRECACHE_MODEL("sprites/svencontra2/bullet_fghit.spr");

        g_Game.PrecacheGeneric( "sprites/svencontra2/hud_sc2fg.spr" );    
        g_Game.PrecacheGeneric( "sprites/svencontra2/bullet_fg.spr" );    
        g_Game.PrecacheGeneric( "sprites/svencontra2/bullet_fghit.spr" );

        g_Game.PrecacheGeneric( "sprites/svencontra2/weapon_sc2fg.txt" );

        CBaseContraWeapon::Precache();
     }
     void CreateProj(int pellet = 1) override{
        CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));
        g_EntityFuncs.SetOrigin( pBullet.self, m_pPlayer->GetGunPosition() );
        *pBullet->pev->owner = *m_pPlayer->edict();
        pBullet->pev->dmg = flDamage;
        pBullet->pev->model = "sprites/svencontra2/bullet_fg.spr";
        //爆炸SPR, 爆炸音效, SPR缩放, 伤害范围, 伤害
        pBullet.SetExpVar("sprites/svencontra2/bullet_fghit.spr", "weapons/svencontra2/shot_fghit.wav", 10, 128, g_WeaponDMG.FGE);
        pBullet->pev->velocity = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES ) * flBulletSpeed;
        pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
        *pBullet.pTouchFunc = *ProjBulletTouch::ExplodeTouch;
        g_EntityFuncs.DispatchSpawn( pBullet.edict() );
    }
 }

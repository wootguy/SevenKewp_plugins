string BULLET_REGISTERNAME = "contra_bullet";
float BULLET_DEFAULT_SPEED = 800;
string BULLET_MDL1 = "sprites/contra/contra_bullet1.spr";
string BULLET_MDL2 = "sprites/contra/contra_bullet2.spr";
float BULLET_DEFAULTDMG = 30;

class CProjBullet : ScriptBaseAnimating
{
    string szSprPath = BULLET_MDL2;
    string szHitSound = "common/null.wav";
    float flSpeed = BULLET_DEFAULT_SPEED;
    float flScale = 0.5f;
    int iDamageType = DMG_BULLET;

    Vector vecHullMin = Vector(-4, -4, -4);
    Vector vecHullMax = Vector(4, 4, 4);

    Vector vecVelocity = Vector(0, 0, 0);
    Vector vecColor = Vector(255, 255, 255);

    void Spawn()
	{	
        if(pev->owner  == NULL )
            return;

        Precache();

		pev.movetype = MOVETYPE_FLYMISSILE;
		pev.solid = SOLID_SLIDEBOX;

        pev->framerate = 1.0f;
        //pev->rendermode = kRenderNormal;
        //pev->renderamt = 255;
        //pev->rendercolor = vecColor;
		
		pev->model = szSprPath;
        pev->scale = flScale;
        pev->speed = flSpeed;
        pev->dmg = BULLET_DEFAULTDMG;

        g_engfuncs.pfnMakeVectors(pev->angles);
		//pev->velocity = gpGlobals->v_forward * pev->speed;
        vecVelocity = pev->velocity;

		SET_MODEL( self, pev->model );
		g_EntityFuncs.SetSize(pev, vecHullMin, vecHullMax);
        g_EntityFuncs.SetOrigin( self, pev->origin );

        pev->nextthink = gpGlobals->time + 0.1f;
	}
    
    void Think()
    {
        pev->velocity = vecVelocity;
        pev->nextthink = gpGlobals->time + 0.1f;
    }

    void SetAnim( int animIndex ) 
	{
		pev->sequence = animIndex;
		pev->frame = 0;
		ResetSequenceInfo();
	}

    void DelayTouch()
    {
        pev->solid = SOLID_SLIDEBOX;
        SetThink( ThinkFunction( this->Think ) );
        pev->nextthink = gpGlobals->time + 0.1f;
    }

    void Precache()
    {
        BaseClass.Precache();
		
        string szTemp = string( pev->model ).IsEmpty() ? szSprPath : string(pev->model);
		PRECACHE_MODEL( szTemp );
        g_Game.PrecacheGeneric( szTemp );

        PRECACHE_MODEL( BULLET_MDL1 );
        g_Game.PrecacheGeneric( BULLET_MDL1 );
        PRECACHE_MODEL( BULLET_MDL2 );
        g_Game.PrecacheGeneric( BULLET_MDL2 );

        PRECACHE_SOUND( szHitSound );
        g_Game.PrecacheGeneric( "sound/" + szHitSound );
    }

    void Touch( CBaseEntity* pOther )
	{
		if( GetClassname() == pOther.GetClassname() || pOther->edict() is pev->owner)
        {
            pev->velocity = gpGlobals->v_forward * pev->speed;
            return;
        }
        
        if(pOther->IsAlive())
            pOther.TakeDamage( pev, pev->owner.vars, pev->dmg, iDamageType);

        g_SoundSystem.EmitSound( edict(), CHAN_AUTO, szHitSound, 1.0f, ATTN_NONE );
        UTIL_Remove(self);
	}
}

CProjBullet* ShootABullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));

    g_EntityFuncs.SetOrigin( pBullet.self, vecOrigin );
    *pBullet->pev->owner = *pOwner->edict();

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
    
    pBullet.SetTouch( TouchFunction( pBullet.Touch ) );

    g_EntityFuncs.DispatchSpawn( pBullet.edict() );

    return pBullet;
}
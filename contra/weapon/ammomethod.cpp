void ShootNormalBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));

    g_EntityFuncs.SetOrigin( pBullet.self, vecOrigin );
    *pBullet->pev->owner = *pOwner->edict();

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
    pBullet.szSprPath = BULLET_MDL1;
    
    pBullet.SetTouch( TouchFunction( pBullet.Touch ) );

    g_EntityFuncs.DispatchSpawn( pBullet.edict() );
}

void ShootMBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));

    g_EntityFuncs.SetOrigin( pBullet.self, vecOrigin );
    *pBullet->pev->owner = *pOwner->edict();

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
    
    pBullet.SetTouch( TouchFunction( pBullet.Touch ) );

    g_EntityFuncs.DispatchSpawn( pBullet.edict() );
}

void ShootSBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    for(uint i = 0; i <= 4; i++)
    {
        CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));
        g_EntityFuncs.SetOrigin( pBullet.self, vecOrigin );
        *pBullet->pev->owner = *pOwner->edict();

        float x, y;
        GetCircularGaussianSpread( x, y );
        MAKE_VECTORS( pOwner->pev->v_angle + pOwner->pev->punchangle );
        Vector vecAngles = gpGlobals->v_forward * vecVelocity.Length() + 
                        (x * 1000 * AMMO_SACCURANCY.x * gpGlobals->v_right + 
                            y * 1000 * AMMO_SACCURANCY.y * gpGlobals->v_up);
        pBullet->pev->velocity = vecAngles;

        pBullet.SetTouch( TouchFunction( pBullet.Touch ) );
        pBullet.SetThink( ThinkFunction( pBullet.DelayTouch ) );
        pBullet->pev->nextthink = gpGlobals->time + 1.0f;
        g_EntityFuncs.DispatchSpawn( pBullet.edict() );
        pBullet->pev->solid = SOLID_NOT;
    }
}

void ShootLBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    for(uint i = 0; i <= 4; i++)
    {
        g_Scheduler.SetTimeout("ShootMBullet", 0.02 * i, *pOwner, vecOrigin, vecVelocity);
    }
}
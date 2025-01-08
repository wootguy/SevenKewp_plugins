string BULLET_REGISTERNAME = "contra_bullet";
string BULLET_HITSOUND = "common/null.wav";

funcdef void BulletTouchCallback( CProjBullet*, CBaseEntity* );
namespace ProjBulletTouch{
    void DefaultDirectTouch(CProjBullet* pThis, CBaseEntity* pOther){
        if(pOther->IsAlive()){
            g_WeaponFuncs.DamageDecal(*pOther, pThis.iDamageType);
            g_WeaponFuncs.SpawnBlood(pThis.pev->origin, pOther.BloodColor(), pThis.pev->dmg);
            pOther.TakeDamage( pThis.pev, pThis.pev->owner.vars, pThis.pev->dmg, pThis.iDamageType);
        }
    }
    void DefaultPostTouch(CProjBullet* pThis, CBaseEntity* pOther){
        g_SoundSystem.EmitSound( pThis.edict(), CHAN_AUTO, pThis.szHitSound, 1.0f, ATTN_NONE );
        UTIL_Remove(pThis.self);
    }
    void DefaultTouch(CProjBullet* pThis, CBaseEntity* pOther){
        ProjBulletTouch::DefaultDirectTouch(*pThis, *pOther);
        ProjBulletTouch::DefaultPostTouch(*pThis, *pOther);
    }
    void ExplodeTouch(CProjBullet* pThis, CBaseEntity* pOther){
        ProjBulletTouch::DefaultDirectTouch(*pThis, *pOther);
        g_WeaponFuncs.RadiusDamage(pThis.pev->origin, pThis.pev, pThis.pev->owner.vars, pThis.flExpDmg, pThis.iExpRadius, -1, pThis.iDamageType);
        MESSAGE_BEGIN m(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
            m.WRITE_BYTE(TE_EXPLOSION);
            m.WRITE_COORD(pThis.pev->origin.x);
            m.WRITE_COORD(pThis.pev->origin.y);
            m.WRITE_COORD(pThis.pev->origin.z);
            m.WRITE_SHORT(g_engfuncs.pfnModelIndex(pThis.szExpSpr));
            m.WRITE_BYTE(pThis.iExpSclae);
            m.WRITE_BYTE(15);
            m.WRITE_BYTE(0);
        m.MESSAGE_END();
        ProjBulletTouch::DefaultPostTouch(*pThis, *pOther);
    }
}
class CProjBullet : ScriptBaseAnimating{
    string szSprPath = "sprites/svencontra2/bullet_mg.spr";
    string szHitSound = BULLET_HITSOUND;
    float flSpeed = 800;
    float flScale = 0.5f;
    int iDamageType = DMG_BULLET;
    Vector vecHullMin = Vector(-4, -4, -4);
    Vector vecHullMax = Vector(4, 4, 4);
    //Exp vars
    string szExpSpr = "sprites/svencontra2/bullet_fghit.spr";
    string szExpSound = "weapons/svencontra2/shot_fghit.wav";
    int iExpSclae;
    int iExpRadius;
    float flExpDmg;

    BulletTouchCallback* pTouchFunc = NULL;

    void SetExpVar(string _s, string _es, int _sc, int _r, float _d){
        szExpSpr = _s;
        szExpSound = _es;
        iExpSclae = _sc;
        iExpRadius = _r;
        flExpDmg = _d;
    }

    void Spawn(){    
        if(pev->owner  == NULL )
            return;
        Precache();
        pev.movetype = MOVETYPE_FLYMISSILE;
        pev.solid = SOLID_TRIGGER;
        pev->framerate = 1.0f;
        if(pev->model == "")
            pev->model = szSprPath;
        if(pev->speed <= 0)
            pev->speed = flSpeed;
        if(pev->dmg <= 0)
            pev->dmg = 30;
        if(pev->scale <= 0)
            pev->scale = flScale;
        if(*pTouchFunc  == NULL )
            *pTouchFunc = *ProjBulletTouch::DefaultTouch;
        pev->rendermode = kRenderTransAdd;
        pev->renderamt = 255;
        pev->rendercolor = Vector(255, 255, 255);
        pev->groupinfo = 114514;
        SET_MODEL( self, pev->model );
        g_EntityFuncs.SetSize(pev, vecHullMin, vecHullMax);
        g_EntityFuncs.SetOrigin( self, pev->origin );
    }

    void SetAnim( int animIndex ) {
        pev->sequence = animIndex;
        pev->frame = 0;
        ResetSequenceInfo();
    }

    void Precache(){
        BaseClass.Precache();
        
        string szTemp = string( pev->model ).IsEmpty() ? szSprPath : string(pev->model);
        PRECACHE_MODEL( szTemp );
        g_Game.PrecacheGeneric( szTemp );

        PRECACHE_MODEL( szExpSpr );
        g_Game.PrecacheGeneric( szExpSpr );

        PRECACHE_MODEL( szSprPath );
        g_Game.PrecacheGeneric( szSprPath );

        PRECACHE_SOUND( szHitSound );
        g_Game.PrecacheGeneric( "sound/" + szHitSound );
         PRECACHE_SOUND( szExpSound );
        g_Game.PrecacheGeneric( "sound/" + szExpSound );
    }
    void Touch( CBaseEntity* pOther ){
        if( pOther.GetClassname() == GetClassname() || pOther->edict() is pev->owner)
            return;
        pTouchFunc(this, pOther);
    }
}

CProjBullet* ShootABullet(edict_t* pOwner, Vector vecOrigin, Vector vecVelocity){
    CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));

    g_EntityFuncs.SetOrigin( pBullet.self, vecOrigin );
    *pBullet->pev->owner = *pOwner;

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
    
    pBullet.SetTouch( TouchFunction( pBullet.Touch ) );

    g_EntityFuncs.DispatchSpawn( pBullet.edict() );

    return pBullet;
}

CProjBullet* ShootABullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity){
    CProjBullet* pBullet = cast<CProjBullet*>(CastToScriptClass(CreateEntity( BULLET_REGISTERNAME, NULL,  false)));

    g_EntityFuncs.SetOrigin( pBullet.self, vecOrigin );
    *pBullet->pev->owner = *pOwner->edict();

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = Math.VecToAngles( pBullet->pev->velocity );
    
    pBullet.SetTouch( TouchFunction( pBullet.Touch ) );

    g_EntityFuncs.DispatchSpawn( pBullet.edict() );

    return pBullet;
}
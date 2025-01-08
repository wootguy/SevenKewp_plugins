const string WEAPONFLAG_REGISTERNAME = "info_weaponflag";
class CWeaponFlag : ScriptBaseAnimating{
    private EHANDLEpOwner = NULL;
    void Spawn(){
        if(*pev->owner  == NULL )
            return;
        pOwner = EHandle(CBaseEntity::Instance(pev->owner));
        Precache();
        pev.movetype = MOVETYPE_NOCLIP;
        pev.solid = SOLID_NOT;
        pev->framerate = 1.0f;
        if(pev->fov <= 0)
            pev->fov = 24;
        pev->scale = 0.25;
        pev->rendermode = kRenderTransAlpha;
        pev->renderamt = 255;
        pev->rendercolor = Vector(255, 255, 255);
        SET_MODEL( self, pev->model );
        g_EntityFuncs.SetOrigin( self, pev->origin );
        pev->nextthink = gpGlobals->time + 0.05f;
    }
    void Think(){
        if(!pOwner.IsValid()){
            SetThink(ThinkFunction(SUB_Remove));
            pev->nextthink = gpGlobals->time;
            return;
        }
        Vector vecOrigin = pOwner.GetEntity()->pev->origin;
        vecOrigin.z += pev->fov;
        g_EntityFuncs.SetOrigin( self, vecOrigin );
        pev->nextthink = gpGlobals->time + 0.05f;
    }
    void Precache(){
        PRECACHE_MODEL( pev->model );
        g_Game.PrecacheGeneric( pev->model );
    }
}
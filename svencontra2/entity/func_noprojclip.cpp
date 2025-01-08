const int SF_TRIG_PUSH_ONCE = 1;
const int SF_TRIGGER_PUSH_START_OFF = 2;
class CNoProjClip : ScriptBaseEntity{
    void Spawn(){
        if ( pev->angles == g_vecZero )
            pev->angles.y = 360;
        if (pev->angles != g_vecZero){
            if (pev->angles == Vector(0, -1, 0))
                pev->movedir = Vector(0, 0, 1);
            else if (pev->angles == Vector(0, -2, 0))
                pev->movedir = Vector(0, 0, -1);
            else{
                MAKE_VECTORS(pev->angles);
                pev->movedir = gpGlobals->v_forward;
            }
            pev->angles = g_vecZero;
        }
        pev->solid = SOLID_TRIGGER;
        pev->movetype = MOVETYPE_NONE;
        SET_MODEL(edict(), pev->model);

        if ( g_engfuncs.pfnCVarGetFloat("showtriggers") == 0 )
            pev->effects |= EF_NODRAW;
        if (pev->speed == 0)
            pev->speed = 100;

        if ( pev->spawnflags & SF_TRIGGER_PUSH_START_OFF != 0 )
            pev->solid = SOLID_NOT;
        g_EntityFuncs.SetOrigin( self, pev->origin );
    }
    void Touch(CBaseEntity* pOther){
        if(!pOther.IsPlayer())
            return;
        entvars_t* pevToucher = pOther.pev;
        switch( pevToucher.movetype ){
            case MOVETYPE_NONE:
            case MOVETYPE_PUSH:
            case MOVETYPE_NOCLIP:
            case MOVETYPE_FOLLOW:
            return;
        }

        if ( pevToucher.solid != SOLID_NOT && pevToucher.solid != SOLID_BSP ){
            if (pev->spawnflags & SF_TRIG_PUSH_ONCE != 0){
                pevToucher.velocity = pevToucher.velocity + (pev->speed * pev->movedir);
                if ( pevToucher.velocity.z > 0 )
                    pevToucher.flags &= ~FL_ONGROUND;
                UTIL_Remove( self );
            }
            else{
                Vector vecPush = (pev->speed * pev->movedir);
                if ( pevToucher.flags & FL_BASEVELOCITY != 0 )
                    vecPush = vecPush +  pevToucher.basevelocity;
                pevToucher.basevelocity = vecPush;
                pevToucher.flags |= FL_BASEVELOCITY;
            }
        }
    }
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue = 0.0f){
        if (pev->solid == SOLID_NOT){
            pev->solid = SOLID_TRIGGER;
            gpGlobals->force_retouch++;
        }
        else
            pev->solid = SOLID_NOT;
        g_EntityFuncs.SetOrigin( self, pev->origin );
    }
    int ObjectCaps() { 
        return BaseClass.ObjectCaps() & ~FCAP_ACROSS_TRANSITION; 
    }
}

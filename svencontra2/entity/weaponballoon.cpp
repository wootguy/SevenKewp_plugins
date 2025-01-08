/*
kSpawnItem 生成物品名称
kReverseTime 气球上下反装时间
kBaloonFloatSpeed 气球上下移动速度
kSprPath 爆炸spr路径
kSprScale 爆炸spr缩放
kSoundPath 爆炸音效路径
kShowName 显示名称
speed 飞行速度
model 模型路径
target info_target名称
*/
//生成时自动触发
const int SF_WEAPONBALLON_STARTSPAWN = 1;

class CWeaponBalloon : ScriptBaseMonsterEntity{
    private bool bInUp = true;
    private string szSpawnItem = "";
    private string szSprPath = "";
    private int iSprScale = 10;
    private string szSoundPath = "";
    private float flDestoryTime;
    private int iFlyReverseTime = 4;
    private float flBaloonUpSpeed = 16.0f;
    private float flInitVelocityZ;
    private CScheduledFunction* pDestoryScheduler = NULL;

    bool KeyValue(const string&szKeyName, const string&szValue){
        if(szKeyName == "kSpawnItem"){
            szSpawnItem = szValue;
            return true;
        }
        else if(szKeyName == "kReverseTime"){
            iFlyReverseTime = atoi(szValue);
            return true;
        }
        else if(szKeyName == "kBaloonFloatSpeed"){
            flBaloonUpSpeed = atof(szValue);
            return true;
        }
        else if(szKeyName == "kSprPath"){
            szSprPath = szValue;
            return true;
        }
        else if(szKeyName == "kSprScale"){
            iSprScale = int(atof(szValue) * 10.0f);
            return true;
        }
        else if(szKeyName == "kSoundPath"){
            szSoundPath = szValue;
            return true;
        }
        else if(szKeyName == "kShowName"){
            m_FormattedName = szValue;
            return true;
        }
        return BaseClass.KeyValue(szKeyName, szValue);
    }
    void Precache(){
        if( string( pev->model ).IsEmpty() )
            PRECACHE_MODEL( "models/common/lambda.mdl" );
        else{
            PRECACHE_MODEL( pev->model );
            g_Game.PrecacheGeneric( pev->model );
        }
        PRECACHE_MODEL( szSprPath );
        g_Game.PrecacheGeneric( szSprPath );
        PRECACHE_SOUND( szSoundPath );
        g_Game.PrecacheGeneric( "sound/" + szSoundPath );
    }
    void Init(){
        CBaseEntity* pEntity = GetNextTarget();
        if(*pEntity  == NULL ){
            UTIL_Remove(self);
            return;
        }
        Vector vecLine = pEntity->pev->origin - pev->origin;
        pev->angles = Math.VecToAngles(vecLine.Normalize());
        flDestoryTime = float(vecLine.Length()) / pev->speed;
        pev->velocity = vecLine.Normalize() * pev->speed;
        flInitVelocityZ = pev->velocity.z;
        pev->velocity.z += flBaloonUpSpeed;
        pev->nextthink = gpGlobals->time + iFlyReverseTime / 2;

        pev->movetype = MOVETYPE_FLY;
        pev->solid = SOLID_SLIDEBOX;
        pev->effects &= ~EF_NODRAW;
        pev->takedamage = DAMAGE_YES;

        pev->health = pev->max_health = 1;

        SET_MODEL( self, string( pev->model ).IsEmpty() ? "models/common/lambda.mdl" : string(pev->model) );
        g_EntityFuncs.SetSize( pev, Vector(-16,-16,-16), Vector(16, 16, 16));

        *pDestoryScheduler = g_Scheduler.SetTimeout(this, "Remove", flDestoryTime);
    }
    void Remove(){
        if(self )
            UTIL_Remove(self);
    }
    void Spawn(){
        if(szSpawnItem.IsEmpty())
            return;
        Precache();

        if(pev->spawnflags & SF_WEAPONBALLON_STARTSPAWN != 0)
            Init();
        else{
            pev->movetype = MOVETYPE_NONE;
            pev->solid = SOLID_NOT;
            pev->effects |= EF_NODRAW;
        }
        g_EntityFuncs.SetOrigin( self, pev->origin );
        BaseClass.Spawn();
    }
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue = 0.0f){
        Init();
    }
    void Think(){
        pev->velocity.z = flInitVelocityZ + (bInUp ? -flBaloonUpSpeed : flBaloonUpSpeed);
        bInUp = !bInUp;      
        pev->nextthink = gpGlobals->time + iFlyReverseTime;
    }
    void Killed(entvars_t* pevAttacker, int iGib){
        BaseClass.Killed(pevAttacker, iGib);

        EMIT_SOUND_DYN( edict(), CHAN_WEAPON, szSoundPath, 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );

        MESSAGE_BEGIN m(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
            m.WRITE_BYTE(TE_EXPLOSION);
            m.WRITE_COORD(pev->origin.x);
            m.WRITE_COORD(pev->origin.y);
            m.WRITE_COORD(pev->origin.z);
            m.WRITE_SHORT(g_engfuncs.pfnModelIndex(szSprPath));
            m.WRITE_BYTE(iSprScale);
            m.WRITE_BYTE(15);
            m.WRITE_BYTE(0);
        m.MESSAGE_END();

        CBaseEntity* pEntity = CreateEntity(szSpawnItem, 
            dictionary = {
                {"origin", pev->origin.ToString()},
                {"angles", pev->angles.ToString()},
                {"m_flCustomRespawnTime", "-1"},
                {"IsNotAmmoItem", "1"}
            }, false);
        *pEntity->pev->owner = edict();
        g_EntityFuncs.DispatchSpawn(pEntity->edict());
        SetThink(NULL);
        g_Scheduler.RemoveTimer(*pDestoryScheduler);
        UTIL_Remove(self);
    }
}
#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"
#include "Scheduler.h"
#include "hlds_hooks.h"

using namespace std;

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

void kill_entity(EHANDLE h_ent) {
    UTIL_Remove(h_ent);
}

class CWeaponBalloon : public CBaseMonster {
public:
    bool bInUp = true;
    string_t szSpawnItem;
    string_t szSprPath;
    int iSprScale = 10;
    string_t szSoundPath;
    float flDestoryTime;
    int iFlyReverseTime = 4;
    float flBaloonUpSpeed = 16.0f;
    float flInitVelocityZ;
    ScheduledFunction pDestoryScheduler = NULL;

    virtual	BOOL IsNormalMonster(void) { return FALSE; }

    void KeyValue(KeyValueData* pkvd)
    {
        if (FStrEq(pkvd->szKeyName, "kSpawnItem"))
        {
            szSpawnItem = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "kReverseTime")) {
            iFlyReverseTime = atoi(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "kBaloonFloatSpeed")) {
            flBaloonUpSpeed = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "kSprPath")) {
            szSprPath = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "kSprScale")) {
            iSprScale = int(atof(pkvd->szValue) * 10.0f);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "kSoundPath")) {
            szSoundPath = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "kShowName")) {
            m_displayName = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else
            CBaseMonster::KeyValue(pkvd);
    }
    void Precache(){
        if( !pev->model)
            PRECACHE_MODEL( "models/common/lambda.mdl" );
        else{
            PRECACHE_MODEL( STRING(pev->model) );
        }
        PRECACHE_MODEL( STRING(szSprPath) );
        PRECACHE_SOUND( STRING(szSoundPath) );
    }
    void Init(){
        CBaseEntity* pEntity = GetNextTarget();
        if(!pEntity){
            UTIL_Remove(this);
            return;
        }
        Vector vecLine = pEntity->pev->origin - pev->origin;
        pev->angles = UTIL_VecToAngles(vecLine.Normalize());
        flDestoryTime = float(vecLine.Length()) / pev->speed;
        pev->velocity = vecLine.Normalize() * pev->speed;
        flInitVelocityZ = pev->velocity.z;
        pev->velocity.z += flBaloonUpSpeed;
        pev->nextthink = gpGlobals->time + iFlyReverseTime / 2;

        //pev->movetype = MOVETYPE_FLY;
        pev->movetype = MOVETYPE_BOUNCE;
        pev->gravity = FLT_MIN;
        pev->friction = 1.0f;

        pev->solid = SOLID_SLIDEBOX;
        pev->effects &= ~EF_NODRAW;
        pev->takedamage = DAMAGE_YES;

        pev->health = pev->max_health = 1;

        SET_MODEL( edict(), !pev->model ? "models/common/lambda.mdl" : STRING(pev->model));
        UTIL_SetSize( pev, Vector(-16,-16,-16), Vector(16, 16, 16));

        pDestoryScheduler = g_Scheduler.SetTimeout(kill_entity, flDestoryTime, EHANDLE(edict()));
    }
    void Spawn(){
        if(!szSpawnItem)
            return;
        Precache();

        if(pev->spawnflags & SF_WEAPONBALLON_STARTSPAWN)
            Init();
        else{
            pev->movetype = MOVETYPE_NONE;
            pev->solid = SOLID_NOT;
            pev->effects |= EF_NODRAW;
        }
        UTIL_SetOrigin( pev, pev->origin );
        CBaseMonster::Spawn();
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
        CBaseMonster::Killed(pevAttacker, iGib);

        EMIT_SOUND_DYN( edict(), CHAN_WEAPON, STRING(szSoundPath), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );

        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
            WRITE_BYTE(TE_EXPLOSION);
            WRITE_COORD(pev->origin.x);
            WRITE_COORD(pev->origin.y);
            WRITE_COORD(pev->origin.z);
            WRITE_SHORT(g_engfuncs.pfnModelIndex(STRING(szSprPath)));
            WRITE_BYTE(iSprScale);
            WRITE_BYTE(15);
            WRITE_BYTE(0);
        MESSAGE_END();

        unordered_map<string, string> keys = {
            {"m_flCustomRespawnTime", "-1"},
            {"IsNotAmmoItem", "1"}
        };
        CBaseEntity* pEntity = CBaseEntity::Create(STRING(szSpawnItem), pev->origin, pev->angles, false, edict(), keys);
        pEntity->pev->owner = edict();
        DispatchSpawn(pEntity->edict());
        SetThink(NULL);
        g_Scheduler.RemoveTimer(pDestoryScheduler);
        UTIL_Remove(this);
    }
};

LINK_ENTITY_TO_CLASS(weaponballoon, CWeaponBalloon)

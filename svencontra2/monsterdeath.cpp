#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"
#include "CSprite.h"
#include "Scheduler.h"

using namespace std;

vector<EHANDLE> aryMonsterList = {};
//spr路径
const string szMonsterDeathSprPath = "sprites/svencontra2/deadspray_a.spr";
//默认音效路径
const string szMosnterDeathSoundPath = "svencontra2/npcdead_a.wav";
//播放spr并删除延迟
const float flMonsterDeathSprRemoveDelay = 0.5f;
//spr速率
const float flMonsterDeathSprFrameRate = 20.0f;
//spr缩放
const float flSprScale = 2.0f;
//向上抬起速度
const float flMonsterDeathLiftSpeed = 200;
//修改重力
const float flMonsterDeathLiftGravity = 0.4;
//渲染模式
//kRenderNormal 0
//kRenderTransColor 1
//kRenderTransTexture 2
//kRenderGlow 3
//kRenderTransAlpha 4
//kRenderTransAdd 5 
const int iMosnterDeathSprRender = 1;
//音频对应表
const unordered_map<string, vector<string>> dicMonsterDeathMap = {
    {"monster_human_grunt", {"sprites/svencontra2/deadspray_a.spr", "svencontra2/npcdead_a.wav"}},
    {"monster_human_grunt_ally", {"sprites/svencontra2/deadspray_a.spr", "svencontra2/npcdead_b.wav"}},
    {"monster_grunt_repel", {"sprites/svencontra2/deadspray_a.spr", "svencontra2/npcdead_a.wav"}},
    {"monster_human_torch_ally", {"sprites/svencontra2/deadspray_a.spr", "svencontra2/npcdead_b.wav"}},
    {"monster_hwgrunt", {"sprites/svencontra2/deadspray_a.spr", "svencontra2/npcdead_b.wav"}},
    {"monster_headcrab", {"sprites/svencontra2/deadspray_c.spr", "svencontra2/npcdead_d.wav"}},
    {"monster_zombie_soldier", {"sprites/svencontra2/deadspray_b.spr", "svencontra2/npcdead_c.wav"}},
    {"monster_handgrenade", {"sprites/svencontra2/mark.spr", "svencontra2/mark.wav"}},
    {"monster_bullchicken", {"sprites/svencontra2/deadspray_b.spr", "svencontra2/npcdead_e.wav"}}
};

void PrecacheAllMonsterDeath(){
    PRECACHE_MODEL_NULLENT( szMonsterDeathSprPath.c_str() );

    PRECACHE_SOUND_NULLENT( szMosnterDeathSoundPath.c_str() );

    for(auto item : dicMonsterDeathMap) {
        PRECACHE_MODEL_NULLENT(item.second[0].c_str());
        PRECACHE_SOUND_NULLENT(item.second[1].c_str());
    }
}

void PlayMonsterDeathSpr(EHANDLE pWho){
    if(!pWho)
        return;
    CBaseEntity* pEntity = pWho.GetEntity();
    string szSpr = szMonsterDeathSprPath;
    string szSound = szMosnterDeathSoundPath;

    auto item = dicMonsterDeathMap.find(STRING(pEntity->pev->classname));
    if(item != dicMonsterDeathMap.end()) {
        szSpr = item->second[0];
        szSound = item->second[1];
    }
    EMIT_SOUND_DYN( pEntity->edict(), CHAN_WEAPON, szSound.c_str(), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));
    
    CSprite* pSpr = CSprite::SpriteCreate(szSpr.c_str(), pEntity->pev->origin, true);
    pSpr->AnimateAndDie(flMonsterDeathSprFrameRate);
    pSpr->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, iMosnterDeathSprRender );
    pSpr->pev->velocity.z = RANDOM_FLOAT(40, 80);
    pSpr->SetScale( flSprScale );
    UTIL_Remove(pEntity);
}

void SearchAndDestoryMonster(){
    for(int i = 0; i < aryMonsterList.size(); i ++){
        if(aryMonsterList[i]){
            if(!aryMonsterList[i]->IsAlive()){
                aryMonsterList[i]->pev->velocity.z += flMonsterDeathLiftSpeed;
                aryMonsterList[i]->pev->gravity = flMonsterDeathLiftGravity;
                g_Scheduler.SetTimeout(PlayMonsterDeathSpr, flMonsterDeathSprRemoveDelay, aryMonsterList[i]);
                aryMonsterList.erase(aryMonsterList.begin() + i);
            }
        }
        else
            aryMonsterList.erase(aryMonsterList.begin() + i);
    }
}

void InitMonsterList(){
    edict_t* pEntity = NULL;
    while(!FNullEnt(pEntity = FIND_ENTITY_BY_CLASSNAME(pEntity, "monster_*")) ){
        CBaseEntity* bent = CBaseEntity::Instance(pEntity);

        if(bent->IsMonster() && bent->IsAlive())
            aryMonsterList.push_back(EHANDLE(pEntity));
    }
}

#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "PluginHooks.h"
#include "dynamicdifficult.h"

using namespace std;

extern vector<EHANDLE> aryMonsterList;

HOOK_RETURN_DATA EntityCreatedHook( CBaseEntity* pEntity ) {
    if(pEntity  == NULL )
        return HOOK_CONTINUE;
    if(pEntity->IsNormalMonster())
        aryMonsterList.push_back(EHANDLE(pEntity->edict()));
    return HOOK_CONTINUE;
}

HOOK_RETURN_DATA ClientPutInServerHook( CBasePlayer* pPlayer ){
    if(pPlayer  == NULL )
        return HOOK_CONTINUE;
    PlayerDMGTweak();
    return HOOK_CONTINUE;
}
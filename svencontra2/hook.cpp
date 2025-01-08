HookReturnCode EntityCreated( CBaseEntity* pEntity ){
    if(pEntity  == NULL )
        return HOOK_CONTINUE;
    if(pEntity.IsMonster())
        aryMonsterList.push_back(EHandle(*pEntity));
    return HOOK_CONTINUE;
}

HookReturnCode ClientPutInServer( CBasePlayer* pPlayer ){
    if(pPlayer  == NULL )
        return HOOK_CONTINUE;
    PlayerDMGTweak();
    return HOOK_CONTINUE;
}
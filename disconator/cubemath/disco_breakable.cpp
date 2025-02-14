
class disco_breakable : ScriptBaseEntity {
	void Precache() {
		BaseClass.Precache();
		
		PRECACHE_SOUND( "debris/metal1.wav" );
		PRECACHE_SOUND( "debris/metal3.wav" );
	}
	
	void Spawn() {
		Precache();
		
		pev->solid = SOLID_BSP;
		pev->movetype = MOVETYPE_PUSH;
		pev->takedamage	= DAMAGE_YES;
		
		SET_MODEL(edict(), pev->model);
	}
	
	void DamageSound(){
		int pitch = PITCH_NORM;
		float fvol = RANDOM_FLOAT(0.75, 1.0);
		vector<string> rgpsz = { "debris/metal1.wav", "debris/metal3.wav"};
		
		if (RANDOM_LONG(0,2) == 0)
			pitch = 95 + RANDOM_LONG(0,34);
		
		EMIT_SOUND_DYN( edict(), CHAN_VOICE, rgpsz[RANDOM_LONG(0,1)], fvol, ATTN_NORM, 0, pitch );
	}
	
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType){
		
		//Only explosives
		if (bitsDamageType == DMG_BLAST && pevAttacker.ClassNameIs( "player" )){
			//Give Player Points for Damaging this->
			if(isBossRunning == 1) pevAttacker.frags += int(ceil(flDamage/10.0f));
		}else{
			return 0;
		}
		
		DamageSound();
		
		return BaseClass.TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}
}

void RegisterDiscoBreakableCustomEntity() {
	g_CustomEntityFuncs.RegisterCustomEntity( "disco_breakable", "disco_breakable" );
}

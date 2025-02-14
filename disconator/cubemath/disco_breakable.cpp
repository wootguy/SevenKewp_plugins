#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"

using namespace std;

extern int isBossRunning;

class CDiscoBreakable : public CBaseEntity {
public:
	void Precache() override {
		PRECACHE_SOUND("debris/metal1.wav");
		PRECACHE_SOUND("debris/metal3.wav");
	}

	void Spawn() override {
		Precache();

		pev->solid = SOLID_BSP;
		pev->movetype = MOVETYPE_PUSH;
		pev->takedamage = DAMAGE_YES;

		SET_MODEL(edict(), STRING(pev->model));
	}

	void DamageSound() {
		int pitch = PITCH_NORM;
		float fvol = RANDOM_FLOAT(0.75, 1.0);
		static const char* rgpsz[2] = { "debris/metal1.wav", "debris/metal3.wav" };

		if (RANDOM_LONG(0, 2) == 0)
			pitch = 95 + RANDOM_LONG(0, 34);

		EMIT_SOUND_DYN(edict(), CHAN_VOICE, rgpsz[RANDOM_LONG(0, 1)], fvol, ATTN_NORM, 0, pitch);
	}

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override {

		//Only explosives
		if (bitsDamageType == DMG_BLAST && (pevAttacker->flags & FL_CLIENT)) {
			//Give Player Points for Damaging this->
			if (isBossRunning == 1) pevAttacker->frags += int(ceil(flDamage / 10.0f));
		}
		else {
			return 0;
		}

		DamageSound();

		return CBaseEntity::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}
};

LINK_ENTITY_TO_CLASS(disco_breakable, CDiscoBreakable)

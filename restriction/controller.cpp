#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "Scheduler.h"

int psychAttackPhase = 0;
float psych_NextAttack = 0;
float psychAttack_Ready = 0;
float psychAuraNextShake = 0;
bool psychAuraSound = false;

EHANDLE controllerH;
EHANDLE controllerTargetH;

void ControllerMapInit()
{
	ALERT( at_console, "Precaching controller's sound\n" );
	PRECACHE_SOUND_NULLENT("russian_sounds/controller/tube_prepare.wav");
	PRECACHE_SOUND_NULLENT("russian_sounds/controller/first_hit.wav");
	PRECACHE_SOUND_NULLENT("russian_sounds/controller/final_hit.wav");
}

Task_t	tlPsychAttack[] =
{
	{ TASK_STOP_MOVING,		(float)0 },	// Move within 128 of target ent (client)
	{ TASK_RANGE_ATTACK1,	(float)0 },
	{ TASK_FACE_ENEMY,		(float)0 },
};

Schedule_t	slPsychAttack[] =
{
	{
		tlPsychAttack,
		ARRAYSIZE(tlPsychAttack),
		0,
		0,
		"PSYCH_ATTACK"
	},
};

void ControllerAttackHit()
{
	if (controllerTargetH)
	{
		CBaseEntity* controllerTarget = controllerTargetH;

		if (controllerH)
		{
			CBaseEntity* controller = controllerH;

			EMIT_SOUND(controllerTarget->edict(), CHAN_ITEM, "russian_sounds/controller/final_hit.wav", 1, ATTN_NORM);
			controllerTarget->TakeDamage(controller->pev, controller->pev, 40, DMG_FALL);

			if (FClassnameIs(controllerTarget->pev, "player")) //Special effects on players
			{
				UTIL_ScreenFade(controllerTarget, Vector(22, 12, 12), 5, 2, 240, 2);
				// g_PlayerFuncs.ScreenFade(controllerTargetM, Vector(62,32,32),5,2,230,0);

				controllerTarget->pev->angles = controllerTarget->pev->angles + Vector(RANDOM_LONG(-100, 100), RANDOM_LONG(-100, 100), 0);
				controllerTarget->pev->fixangle = FAM_FORCEVIEWANGLES;
			}
		}
	}
}

/* npc_controller
Controller's psychic attacks
Uses trigger_script in think mode :
target : Controller's targetname
Time between thinks : 0.5 (Will affect the frequency of psychic aura damage)
The trigger_script must be deactivated at the controller's death
*/
void npc_controller(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
{
	CBaseEntity* scriptTarget = UTIL_FindEntityByTargetname( NULL, STRING(pCaller->pev->target) );
	CBaseMonster* controller = scriptTarget->MyMonsterPointer();
	controllerH = controller;
	
	CBaseEntity* controllerTarget = controller->m_hEnemy;
	CBaseMonster* controllerTargetM = controllerTarget ? controllerTarget->MyMonsterPointer() : NULL;
	
	//Psychic attack
	bool controllerSeeTarget = (controllerTarget != NULL) && (controller->HasConditions(bits_COND_SEE_ENEMY) == true) && ( (controller->pev->origin - controllerTarget->pev->origin).Length() < 1350 );
	bool controllerCanSeeTarget = (controllerTarget != NULL) && (controller->HasConditions(bits_COND_ENEMY_OCCLUDED) == false) && ( (controller->pev->origin - controllerTarget->pev->origin).Length() < 1350 );
	
	switch (psychAttackPhase)
	{
	case 0: //No psychic attack started
	
		//Can try to attack if ready
		if (gpGlobals->time > psych_NextAttack && controller->m_Activity != ACT_MELEE_ATTACK1 && controller->m_Activity != ACT_SMALL_FLINCH && controller->m_Activity != ACT_BIG_FLINCH)
		{
		//Will attack on sight || random chance to attack if target is visible but currently not in field of view
			if (controllerSeeTarget || (controllerCanSeeTarget && RANDOM_LONG(1,100)>10)) 
			{
			controller->ChangeSchedule(slPsychAttack);
			EMIT_SOUND(controllerTarget->edict(), CHAN_ITEM, "russian_sounds/controller/tube_prepare.wav",0.8,ATTN_NORM);
			psychAttackPhase = 1;
			psychAttack_Ready = gpGlobals->time + 1.2;
			}
		}
		
		break;
		
	case 1: //Loading psychic attack
		if (controllerSeeTarget && gpGlobals->time > psychAttack_Ready && gpGlobals->time < (psychAttack_Ready + 2) && controller->m_Activity == ACT_RANGE_ATTACK1)
		{
			EMIT_SOUND(controllerTarget->edict(), CHAN_ITEM, "russian_sounds/controller/first_hit.wav",1,ATTN_NORM);
			controller->SetSequenceByName("psychattack_hit");
			
			 //Camera effect on players
			if (controllerTargetM->IsPlayer())
			{
				Vector vecToTarget = (controller->pev->origin - controllerTargetM->pev->origin + Vector(0,0,48));
						
				Vector psychCamAngles = UTIL_VecToAngles(vecToTarget * Vector(1,1,-1));
				Vector psychCamOrigin = controller->pev->origin + Vector(0,0,64) - vecToTarget.Normalize() * 55;

				std::unordered_map<std::string, std::string> keys = {
					{"wait", "1" }
				};
				CBaseEntity* psychCam = CBaseEntity::Create("trigger_camera", psychCamOrigin, psychCamAngles,
					true, NULL, keys);

				psychCam->pev->spawnflags = 4;
			
			
				psychCam->Use(controllerTargetM,controllerTargetM,USE_ON);
				UTIL_ScreenFade(controllerTargetM, Vector(255,255,255),0.8,0.2,150,1);
			}
			
			//Scheduling last attack phase
			controllerTargetH = controllerTargetM;
			g_Scheduler.SetTimeout(ControllerAttackHit,1);

			psychAttackPhase = 0;
			psych_NextAttack = gpGlobals->time + 4;
		}

		else if (gpGlobals->time > (psychAttack_Ready + 2)) //Attack expiration
		{
			psychAttackPhase = 0;
			psych_NextAttack = gpGlobals->time + 1;
		}
		break;
	}
	
		
	//Psychic aura effects
	CBaseEntity* playerInAura = NULL;
	CBaseEntity* NPCInAura = NULL;
	
	//Damage on players
	while ((playerInAura = UTIL_FindEntityInSphere(playerInAura, controller->pev->origin, 512)) != NULL)
	{
		if (!playerInAura->IsPlayer()) {
			continue;
		}

		CBaseMonster* playerInAuraM = playerInAura->MyMonsterPointer();
		float psychAuraDmg = 4 * (1 - (controller->pev->origin - playerInAuraM->pev->origin).Length() / 512);
		playerInAuraM->TakeDamage(controller->pev, controller->pev, psychAuraDmg, DMG_FALL);
	}
	
	//Mind control on "controllable" NPCs (Defined by targetname)
	while ((NPCInAura = UTIL_FindEntityInSphere(NPCInAura, controller->pev->origin, 512)) != NULL)
	{
		if (strcmp(STRING(NPCInAura->pev->targetname), "controllable")) {
			continue;
		}

		CBaseMonster* NPCInAuraM = NPCInAura->MyMonsterPointer();
		NPCInAuraM->SetClassification(CLASS_ALIEN_MONSTER);
		NPCInAuraM->pev->targetname = 0;
	}

	
	
	//Psychic aura shake
	if (gpGlobals->time >= psychAuraNextShake)
	{
	UTIL_ScreenShake(controller->pev->origin, 10, 10, 3, 510);
	psychAuraNextShake = gpGlobals->time + 3;
	}

}

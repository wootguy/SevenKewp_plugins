int psychAttackPhase = 0;
float psych_NextAttack = 0;
float psychAttack_Ready = 0;
float psychAuraNextShake = 0;
bool psychAuraSound = false;

EHANDLEcontrollerH;
EHANDLEcontrollerTargetH;

void ControllerMapInit()
{
	g_Game.AlertMessage( at_console, "Precaching controller's sound\n" );
	PRECACHE_SOUND("russian_sounds/controller/tube_prepare.ogg");
	PRECACHE_SOUND("russian_sounds/controller/first_hit.ogg");
	PRECACHE_SOUND("russian_sounds/controller/final_hit.ogg");
}


/* npc_controller
Controller's psychic attacks
Uses trigger_script in think mode :
target : Controller's targetname
Time between thinks : 0.5 (Will affect the frequency of psychic aura damage)
The trigger_script must be deactivated at the controller's death
*/
void npc_controller(CBaseEntity* controllerScript)
{
	CBaseEntity* scriptTarget = g_EntityFuncs.FindEntityByTargetname( NULL, controllerScript->pev->target );
	CBaseMonster* controller = cast<CBaseMonster*>(scriptTarget);
	controllerH = controller;
	
	CBaseEntity* controllerTarget = controller.m_hEnemy;
	CBaseMonster* controllerTargetM = cast<CBaseMonster*>(controllerTarget);
	
	//Psychic attack schedule
	ScriptSchedule* psychAttack = ScriptSchedule(0, 0, "psychAttack");
	
	ScriptTask psychAttack_T1;	psychAttack_T1.iTask = TASK_STOP_MOVING;
	ScriptTask psychAttack_T2;	psychAttack_T2.iTask = TASK_RANGE_ATTACK1;
	ScriptTask psychAttack_T3;	psychAttack_T3.iTask = TASK_FACE_ENEMY;
	
	psychAttack.AddTask(psychAttack_T1);
	psychAttack.AddTask(psychAttack_T2);
	psychAttack.AddTask(psychAttack_T3);
	
	Schedule* psychAttackSched = psychAttack.opImplCast();
	
	//Psychic attack
	bool controllerSeeTarget = (*controllerTarget != NULL) && (controller.HasConditions(bits_COND_SEE_ENEMY) == true) && ( (controller->pev->origin - controllerTarget->pev->origin).Length() < 1350 );
	bool controllerCanSeeTarget = (*controllerTarget != NULL) && (controller.HasConditions(bits_COND_ENEMY_OCCLUDED) == false) && ( (controller->pev->origin - controllerTarget->pev->origin).Length() < 1350 );
	
	switch (psychAttackPhase)
	{
	case 0: //No psychic attack started
	
		//Can try to attack if ready
		if (gpGlobals->time > psych_NextAttack && controller.m_Activity != ACT_MELEE_ATTACK1 && controller.m_Activity != ACT_SMALL_FLINCH && controller.m_Activity != ACT_BIG_FLINCH)
		{
		//Will attack on sight || random chance to attack if target is visible but currently not in field of view
			if (controllerSeeTarget || (controllerCanSeeTarget && RANDOM_LONG(1,100)>10)) 
			{
			controller.ChangeSchedule(psychAttackSched);
			EMIT_SOUND(controllerTarget->edict(), CHAN_ITEM, "russian_sounds/controller/tube_prepare.ogg",0.8,ATTN_NORM);
			psychAttackPhase = 1;
			psychAttack_Ready = gpGlobals->time + 1.2;
			}
		}
		
		break;
		
	case 1: //Loading psychic attack
		if (controllerSeeTarget && gpGlobals->time > psychAttack_Ready && gpGlobals->time < (psychAttack_Ready + 2) && controller.m_Activity == ACT_RANGE_ATTACK1)
		{
			EMIT_SOUND(controllerTarget->edict(), CHAN_ITEM, "russian_sounds/controller/first_hit.ogg",1,ATTN_NORM);
			controller.SetSequenceByName("psychattack_hit");
			
			 //Camera effect on players
			if (controllerTargetM->pev->classname == "player")
			{
			Vector vecToTarget = (controller->pev->origin - controllerTargetM->pev->origin + Vector(0,0,48));
						
			Vector psychCamAngles = UTIL_VecToAngles(vecToTarget * Vector(1,1,-1));
			Vector psychCamOrigin = controller->pev->origin + Vector(0,0,64) - vecToTarget * 55 / vecToTarget.Length();

			CBaseEntity* psychCam = g_EntityFuncs.Create("trigger_camera", psychCamOrigin, psychCamAngles,false);

			psychCam->pev->spawnflags = 4;
			psychCam.KeyValue("wait","1");
			
			
			psychCam.Use(*controllerTargetM,*controllerTargetM,USE_ON);
			g_PlayerFuncs.ScreenFade(controllerTargetM, Vector(255,255,255),0.8,0.2,150,1);
			}
			
			//Scheduling last attack phase
			controllerTargetH = controllerTargetM;
			g_Scheduler.SetTimeout("ControllerAttackHit",1);

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
	while ((*playerInAura = g_EntityFuncs.FindEntityInSphere(playerInAura, controller->pev->origin, 512, "player", "classname")) != NULL)
	{
		CBaseMonster* playerInAuraM =  cast<CBaseMonster*>(*playerInAura);
		float psychAuraDmg = 4 * (1 - (controller->pev->origin - playerInAuraM->pev->origin).Length() / 512);
		playerInAuraM.TakeDamage(controller.pev, controller.pev, psychAuraDmg, DMG_FALL);
	}
	
	//Mind control on "controllable" NPCs (Defined by targetname)
	while ((*NPCInAura = g_EntityFuncs.FindEntityInSphere(NPCInAura, controller->pev->origin, 512, "controllable", "targetname")) != NULL)
	{
		CBaseMonster* NPCInAuraM = cast<CBaseMonster*>(*NPCInAura);
		NPCInAuraM.SetClassification(CLASS_ALIEN_MONSTER);
		NPCInAuraM->pev->targetname = "";
	}

	
	
	//Psychic aura shake
	if (gpGlobals->time >= psychAuraNextShake)
	{
	g_PlayerFuncs.ScreenShake(controller->pev->origin, 10, 10, 3, 510);
	psychAuraNextShake = gpGlobals->time + 3;
	}

}

void ControllerAttackHit()
{
	if(controllerTargetH)
	{
		CBaseEntity* controllerTarget = controllerTargetH;
		
		if (controllerH)
		{
			CBaseEntity* controller = controllerH;
		
			EMIT_SOUND(controllerTarget->edict(), CHAN_ITEM, "russian_sounds/controller/final_hit.ogg",1,ATTN_NORM);
			controllerTarget.TakeDamage(controller.pev, controller.pev, 40, DMG_FALL);
			
			if (controllerTarget->pev->classname == "player") //Special effects on players
			{		
				g_PlayerFuncs.ScreenFade(controllerTarget, Vector(22,12,12),5,2,240,2);
				// g_PlayerFuncs.ScreenFade(controllerTargetM, Vector(62,32,32),5,2,230,0);
				
				controllerTarget->pev->angles = controllerTarget->pev->angles + Vector(RANDOM_LONG(-100,100),RANDOM_LONG(-100,100),0);
				controllerTarget->pev->fixangle = FAM_FORCEVIEWANGLES;
			}
		}
	}
}
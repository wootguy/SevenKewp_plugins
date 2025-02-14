#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "hlds_hooks.h"
#include "Scheduler.h"
#include "explode.h"

enum DisconatorAnimation
{
	DISCONATOR_IDLE = 0,
	DISCONATOR_BOMBL,
	DISCONATOR_BOMBR,
	DISCONATOR_BOMBB,
	DISCONATOR_RAGE,
	DISCONATOR_DIE
};

using namespace std;

float g_musicBeginTime;
int isBossRunning;

// There is a delay between "mp3" commands and music playback in Half-Life.
// This keeps music in sync with the disco floor beeps (until you alt-tab
// or the server lags). -w00tguy
const float musicLag = -0.75f;

class disco_player {
public:
	CBasePlayer* m_pPlayer = NULL;
	vector<Vector> oldPos;

	disco_player() {}

	disco_player(CBasePlayer* pPlayer) {
		m_pPlayer = pPlayer;

		oldPos.resize(32);

		for (int i = 0; i < 32; ++i) {
			oldPos[i] = pPlayer->pev->origin;
		}
	}

	void updatePos(CBasePlayer* pPlayer) {
		if (pPlayer == NULL || !IsValidPlayer(pPlayer->edict())) {
			m_pPlayer = NULL;
		}
		else {
			for (int i = 31; i > 0; --i) {
				oldPos[i] = oldPos[i - 1];
			}
			oldPos[0] = pPlayer->pev->origin;
		}
	}
};
vector<disco_player> g_disco_player;

class disco_aim_cross;
void delay_calc_position(disco_aim_cross* cross);

class disco_aim_cross {
public:
	CBasePlayer* pTargetPlayer = NULL;
	Vector targetPos = Vector(0,0,0);
	int playerIndex = 0;

	disco_aim_cross() {}

	void ShufflePlayer() {
		float sumHealth = 0.0f;

		CBasePlayer* pPlayer = NULL;
		pTargetPlayer = NULL;
		playerIndex = -1;
		for (int iPlayer = 1; iPlayer <= gpGlobals->maxClients; ++iPlayer) {
			pPlayer = UTIL_PlayerByIndex(iPlayer);

			if (pPlayer == NULL || !pPlayer->IsAlive())
				continue;

			sumHealth += pPlayer->pev->health;
		}

		float randomHealthPick = RANDOM_FLOAT(0.0f, sumHealth);

		sumHealth = 0.0f;
		for (int iPlayer = 1; iPlayer <= gpGlobals->maxClients; ++iPlayer) {
			pPlayer = UTIL_PlayerByIndex(iPlayer);

			if (pPlayer == NULL || !pPlayer->IsAlive())
				continue;

			sumHealth += pPlayer->pev->health;
			if (randomHealthPick <= sumHealth) {
				pTargetPlayer = pPlayer;
				playerIndex = iPlayer - 1;
				break;
			}
		}
	}

	void ThinkResizeArray() {
		g_disco_player.resize(gpGlobals->maxClients);
		CBasePlayer* pPlayer = NULL;
		for (int iPlayer = 1; iPlayer <= gpGlobals->maxClients; ++iPlayer) {
			pPlayer = UTIL_PlayerByIndex(iPlayer);

			if (pPlayer == NULL)
				continue;

			disco_player data(pPlayer);
			g_disco_player[pPlayer->entindex() - 1] = data;
		}

		ShufflePlayer();
		CalcPosition();
	}

	void CalcPosition() {
		if (g_disco_player.size() != (uint32_t)(gpGlobals->maxClients)) {
			ThinkResizeArray();
		}
		else {
			CBasePlayer* pPlayer = NULL;

			for (int iPlayer = 1; iPlayer <= gpGlobals->maxClients; ++iPlayer) {
				pPlayer = UTIL_PlayerByIndex(iPlayer);

				if (pPlayer == NULL || !pPlayer->IsAlive())
					continue;

				if (g_disco_player[pPlayer->entindex() - 1].m_pPlayer == NULL) {
					disco_player data(pPlayer);
					g_disco_player[pPlayer->entindex() - 1] = data;
				}
				else {
					g_disco_player[pPlayer->entindex() - 1].updatePos(pPlayer);
				}
			}

			if (pTargetPlayer == NULL || !pTargetPlayer->IsAlive()) {
				targetPos.x = 0.0f;
				targetPos.y = 0.0f;

				ShufflePlayer();
			}
			else {
				disco_player& pTargetPlayerData = g_disco_player[playerIndex];

				Vector posA = pTargetPlayerData.oldPos[31];
				Vector posC = pTargetPlayerData.oldPos[0];
				Vector posAC = posC - posA;

				targetPos.x = posAC.x * 0.5f + posC.x;
				targetPos.y = posAC.y * 0.5f + posC.y;
			}

			g_Scheduler.SetTimeout(delay_calc_position, 0.1f, this);
		}
	}
};

void delay_calc_position(disco_aim_cross* cross) {
	cross->CalcPosition();
}

disco_aim_cross g_disco_aim_cross;

class CDiscoHealthLogic : public CBaseEntity {
public:
	
	CBaseEntity* gloVariables;
	CBaseEntity* bossHealthBox;
	CBaseEntity* messageEntity;
	CBaseEntity* disconatorEntity;
	
	int laserLevel;
	int bossLevel;
	float mapEntTimer;
	
	void Spawn() {
		Precache();
		
		for( int i = 0; i < gpGlobals->maxEntities; ++i ) {
			CBaseEntity* pEntity = CBaseEntity::Instance( INDEXENT(i) );
			
			if( pEntity ){
				if( !strcmp(STRING(pEntity->pev->targetname), "variables")) {
					gloVariables = pEntity;
				}
				if(!strcmp(STRING(pEntity->pev->targetname), "boss_health")) {
					bossHealthBox = pEntity;
				}
				if(!strcmp(STRING(pEntity->pev->classname), "disco_disconator")) {
					disconatorEntity = pEntity;
				}
			}
		}
		
		unordered_map<string, string> keyvalues = {
			{"message", "Health: 100.0"},
			{"targetname", "show_health"},
			{"delay", "0"},
			{"x", "-1"},
			{"y", "0.1"},
			{"effect", "0"},
			{"color", "255 255 255"},
			{"color2", "240 110 0"},
			{"fadein", "0.0"},
			{"fadeout", "0.5"},
			{"holdtime", "60"},
			{"fxtime", "0.25"},
			{"channel", "1"},
			{"spawnflags", "3"}
		};
		
		messageEntity = CBaseEntity::Create( "game_text", g_vecZero, g_vecZero, true, NULL, keyvalues );
		
		SetThink( &CDiscoHealthLogic::Think01 );
		pev->nextthink = gpGlobals->time + 0.25f;
		laserLevel = 0;
		isBossRunning = 0;
		bossLevel = 1;
	}
	
	void Think01(){
		int bossmode = (*gloVariables->GetCustomKeyValues())["$i_bossmode"].iVal;
		
		if(bossmode == 1){
			g_musicBeginTime = 0.0f;
			isBossRunning = 1;
			
			SetThink( &CDiscoHealthLogic::Think02 );
			if(bossHealthBox ){
				bossHealthBox->pev->health = 100000.0f;
			}
		}
		
		pev->nextthink = gpGlobals->time + 0.25f;
	}
	
	void Think02(){
		unordered_map<string, CKeyValue>& keyValues1 = *gloVariables->GetCustomKeyValues();
		float maxHealth = keyValues1["$f_maxhealth"].fVal;
		float health = keyValues1["$f_health"].fVal;
		float healthPercentage = keyValues1["$f_health_percentage"].fVal;
		
		if(bossHealthBox ){
			health -= 100000.0f - bossHealthBox->pev->health;
			bossHealthBox->pev->health = 100000.0f;
		}
		DispatchKeyValue(gloVariables->edict(), "$f_health", health);
		
		healthPercentage = health / maxHealth * 100.0f;
		if(healthPercentage > 0.0f && healthPercentage < 0.1f) healthPercentage = 0.1f;
		if(healthPercentage > 100.0f) healthPercentage = 100.0f;
		if(healthPercentage < 0.0f) healthPercentage = 0.0f;
		
		DispatchKeyValue(gloVariables->edict(), "$f_health_percentage", healthPercentage);
		
		if(laserLevel == 0 && healthPercentage <= 50.0f){
			FireTargets("laser_blue_1_harmless", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("laser_blue_1_harmless", NULL, NULL, USE_OFF, 0.0f, 3.0f);
			FireTargets("laser_blue_1", NULL, NULL, USE_ON, 0.0f, 3.0f);
			FireTargets("laser_orange_1_harmless", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("laser_orange_1_harmless", NULL, NULL, USE_OFF, 0.0f, 3.0f);
			FireTargets("laser_orange_1", NULL, NULL, USE_ON, 0.0f, 3.0f);
			laserLevel = 1;
		}
		
		if(laserLevel == 1 && healthPercentage <= 25.0f){
			FireTargets("laser_blue_2_harmless", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("laser_blue_2_harmless", NULL, NULL, USE_OFF, 0.0f, 3.0f);
			FireTargets("laser_blue_2", NULL, NULL, USE_ON, 0.0f, 3.0f);
			FireTargets("laser_orange_2_harmless", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("laser_orange_2_harmless", NULL, NULL, USE_OFF, 0.0f, 3.0f);
			FireTargets("laser_orange_2", NULL, NULL, USE_ON, 0.0f, 3.0f);
			FireTargets("final_alert", NULL, NULL, USE_ON, 0.0f, 0.0f);
			laserLevel = 2;
		}
		
		pev->nextthink = gpGlobals->time + 0.25f;
		
		if(healthPercentage <= 0.0f && bossLevel == 4) {
			CBaseEntity* pEnt = CBaseEntity::Create("disco_disconator_dying", disconatorEntity->pev->origin, disconatorEntity->pev->angles, true);
			UTIL_Remove( disconatorEntity );
			UTIL_Remove( bossHealthBox );
			disconatorEntity = pEnt;
			bossLevel = 5;
			FireTargets("laser_blue_1", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("laser_orange_1", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("laser_blue_2", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("laser_orange_2", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("light_safe", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("light_dangerous", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("final_alert", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("music4", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("exploding_boss_1", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("exploding_boss_6", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("exploding_boss_4", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("exploding_boss_9", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("exploding_boss_1", NULL, NULL, USE_ON, 0.0f, 0.7f);
			FireTargets("exploding_boss_2", NULL, NULL, USE_ON, 0.0f, 1.1f);
			FireTargets("exploding_boss_3", NULL, NULL, USE_ON, 0.0f, 1.3f);
			FireTargets("exploding_boss_4", NULL, NULL, USE_ON, 0.0f, 1.4f);
			FireTargets("exploding_boss_5", NULL, NULL, USE_ON, 0.0f, 1.5f);
			FireTargets("exploding_boss_6", NULL, NULL, USE_ON, 0.0f, 2.0f);
			FireTargets("exploding_boss_7", NULL, NULL, USE_ON, 0.0f, 2.2f);
			FireTargets("exploding_boss_8", NULL, NULL, USE_ON, 0.0f, 2.4f);
			FireTargets("exploding_boss_9", NULL, NULL, USE_ON, 0.0f, 2.5f);
			FireTargets("exploding_boss_1", NULL, NULL, USE_ON, 0.0f, 2.8f);
			FireTargets("exploding_boss_2", NULL, NULL, USE_ON, 0.0f, 2.9f);
			FireTargets("exploding_boss_3", NULL, NULL, USE_ON, 0.0f, 3.0f);
			FireTargets("exploding_boss_4", NULL, NULL, USE_ON, 0.0f, 3.3f);
			FireTargets("exploding_boss_5", NULL, NULL, USE_ON, 0.0f, 3.5f);
			FireTargets("exploding_boss_6", NULL, NULL, USE_ON, 0.0f, 3.6f);
			FireTargets("exploding_boss_7", NULL, NULL, USE_ON, 0.0f, 4.0f);
			FireTargets("exploding_boss_8", NULL, NULL, USE_ON, 0.0f, 4.2f);
			FireTargets("exploding_boss_9", NULL, NULL, USE_ON, 0.0f, 4.3f);
			FireTargets("exploding_boss_10", NULL, NULL, USE_ON, 0.0f, 5.0f);
			FireTargets("exploding_boss_1", NULL, NULL, USE_ON, 0.0f, 5.0f);
			FireTargets("exploding_boss_6", NULL, NULL, USE_ON, 0.0f, 5.0f);
			FireTargets("exploding_boss_4", NULL, NULL, USE_ON, 0.0f, 5.0f);
			FireTargets("exploding_boss_9", NULL, NULL, USE_ON, 0.0f, 5.0f);
			FireTargets("boss_metal", NULL, NULL, USE_ON, 0.0f, 5.0f);
			
			mapEntTimer = gpGlobals->time + 15.0f;
			pev->nextthink = gpGlobals->time + 10.0f;
			SetThink( &CDiscoHealthLogic::Think03 );
		}else if(healthPercentage <= 25.0f && bossLevel == 3) {
			CBaseEntity* pEnt = CBaseEntity::Create("disco_disconator4", disconatorEntity->pev->origin, disconatorEntity->pev->angles, true);
			UTIL_Remove( disconatorEntity );
			disconatorEntity = pEnt;
			bossLevel = 4;
			FireTargets("light_dangerous", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("light_normal", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("boss_glass", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("music3", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("music4", NULL, NULL, USE_ON, 0.0f, 1.0f + musicLag);
			g_musicBeginTime = gpGlobals->time + 1.093f;
		}else if(healthPercentage <= 50.0f && bossLevel == 2) {
			CBaseEntity* pEnt = CBaseEntity::Create("disco_disconator3", disconatorEntity->pev->origin, disconatorEntity->pev->angles, true);
			UTIL_Remove( disconatorEntity );
			disconatorEntity = pEnt;
			bossLevel = 3;
			FireTargets("disco_beams", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("phase_2_breakable", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("music2", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("music3", NULL, NULL, USE_ON, 0.0f, 1.0f + musicLag);
			g_musicBeginTime = gpGlobals->time + 1.450f;
		}else if(healthPercentage <= 75.0f && bossLevel == 1) {
			CBaseEntity* pEnt = CBaseEntity::Create("disco_disconator2", disconatorEntity->pev->origin, disconatorEntity->pev->angles, true);
			UTIL_Remove( disconatorEntity );
			disconatorEntity = pEnt;
			bossLevel = 2;
			FireTargets("disco_beams", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("phase_1_breakable", NULL, NULL, USE_ON, 0.0f, 0.0f);
			FireTargets("music1", NULL, NULL, USE_OFF, 0.0f, 0.0f);
			FireTargets("music2", NULL, NULL, USE_ON, 0.0f, 1.0f + musicLag);
		}
		
		DispatchKeyValue(messageEntity->edict(), "message", UTIL_VarArgs("Health: %.1f", healthPercentage));
		
		FireTargets("show_health", NULL, NULL, USE_ON);
	}
	
	void Think03(){
		bool arePeopleAlife = false;
		
		CBasePlayer* pPlayer = NULL;
		for( int iPlayer = 1; iPlayer <= gpGlobals->maxClients; ++iPlayer ){
			pPlayer = UTIL_PlayerByIndex( iPlayer );
		   
			if( pPlayer  == NULL || !pPlayer->IsAlive())
				continue;
			
			arePeopleAlife = true;
			break;
		}
		
		if(arePeopleAlife){
			DispatchKeyValue(messageEntity->edict(), "message", "Thank you for playing my map. Map by CubeMath.");
			if(mapEntTimer < gpGlobals->time){
				SetThink( &CDiscoHealthLogic::Think04 );
			}
		}else{
			DispatchKeyValue(messageEntity->edict(), "message", "You beat the Boss && yet failed to Survive. Game Over.");
		}
		
		FireTargets("show_health", NULL, NULL, USE_ON);
		
		pev->nextthink = gpGlobals->time + 0.25f;
	}
	
	void Think04(){
		FireTargets("end_of_the_line", NULL, NULL, USE_ON, 0.0f, 0.0f);
	}
};

class CDiscoFireball : public CBaseEntity {
public:
	void Spawn() {
		Precache();

		pev->movetype = MOVETYPE_NOCLIP;
		pev->solid = SOLID_NOT;
		pev->framerate = 1.0f;
		pev->rendermode = 5;
		pev->renderamt = 255.0f;
		pev->scale = 0.25f;
		pev->gravity = 0.75f;

		SET_MODEL(edict(), "sprites/particlesprite.spr");

		//Unlikely that the fireball needs >10s to touch floor.
		pev->nextthink = gpGlobals->time + 0.125f;
		SetThink(&CDiscoFireball::SolidThink);
		SetTouch(&CDiscoFireball::DetoTouch);
	}

	void SolidThink() {
		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_BBOX;
		UTIL_SetSize(pev, Vector(-8, -8, 0), Vector(8, 8, 8));

		pev->nextthink = gpGlobals->time + 10.0f;
		SetThink(&CDiscoFireball::DetoThink);
	}

	void DetoThink() {
		ExplosionCreate(pev->origin, Vector(0, 0, 0), NULL, 49, true);
		UTIL_Remove(this);
	}

	void DetoTouch(CBaseEntity* pOther) {
		DetoThink();
	}
};

class CDiscoDisconator : public CBaseAnimating {
public:
	CBaseEntity* gloVariables2;
	
	void Precache() {		
		PRECACHE_MODEL( "models/cubemath/arrow2d.mdl" );
		PRECACHE_MODEL( "models/cubemath/discoboss/disconator.mdl" );
		PRECACHE_MODEL( "models/cubemath/discoboss/disconatorbroken.mdl" );
		PRECACHE_MODEL( "sprites/particlesprite.spr" );
	}
	
	// if youre gonna use this in your script, make sure you dont try to access invalid animations -zode
	void SetAnim(int animIndex) {
		pev->sequence = animIndex;
		pev->frame = 0;
		ResetSequenceInfo();
	}
	
	void Spawn() {
		Precache();
		
		SET_MODEL(edict(), "models/cubemath/discoboss/disconator.mdl");
		UTIL_SetOrigin(pev, pev->origin);
		
		pev->targetname = MAKE_STRING("disconator");
		pev->framerate = 1.0f;
		
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;

		SetAnim(DISCONATOR_IDLE);
		
		SetThink( &CDiscoDisconator::ThinkPrepareBattle );
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	
	void ThinkPrepareBattle(){
		CBaseEntity::Create("disco_health_logic", pev->origin, Vector(0, 0, 0), true);
		
		for( int i = 0; i < gpGlobals->maxEntities; ++i ) {
			CBaseEntity* pEntity = CBaseEntity::Instance( INDEXENT(i) );
			
			if( pEntity ){
				if( !strcmp(STRING(pEntity->pev->targetname), "variables")) {
					gloVariables2 = pEntity;
					break;
				}
			}
		}
		
		SetThink( &CDiscoDisconator::ThinkWait );
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	
	void ThinkWait(){
		int bossmode = (*gloVariables2->GetCustomKeyValues())["$i_bossmode"].iVal;
		
		if(bossmode == 1){
			g_disco_aim_cross.ThinkResizeArray();
			SetThink( &CDiscoDisconator::ThinkRotateChooseDirection );
			
			FireTargets("music1", NULL, NULL, USE_ON, 0.0f, 0.0f);
		}
		
		pev->nextthink = gpGlobals->time + 0.25f;
	}
	
	void ThinkRotateChooseDirection(){
		
		SetAnim(DISCONATOR_IDLE);
		
		float pHealth = (*gloVariables2->GetCustomKeyValues())["$f_health_percentage"].fVal;
		
		if(pHealth > 90.0f){
			if(RANDOM_LONG( 0, 1 ) == 1){
				pev->avelocity.y = -30.0f;
				SetThink( &CDiscoDisconator::ThinkBreak1R );
			}else{
				pev->avelocity.y =  30.0f;
				SetThink( &CDiscoDisconator::ThinkBreak1L );
			}
			pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.0f, 3.0f );
		}else{
			g_disco_aim_cross.ShufflePlayer();
			
			float currentRotation = pev->angles.y / 180.0f * 3.14159265358979323846f;
			float pathLength = sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y);
			if(pathLength == 0.0f){
				ThinkThrowBombRight1();
			}else{
				float targetRotation = 0.0f;
				if(g_disco_aim_cross.targetPos.y < 0.0f) targetRotation = acos(-g_disco_aim_cross.targetPos.x/pathLength) - 3.14159265358979323846f;
				else targetRotation = acos(g_disco_aim_cross.targetPos.x/pathLength);
				
				currentRotation -= targetRotation;
				while(currentRotation < 0.0f) currentRotation += 3.14159265358979323846f * 2.0f;
				while(currentRotation > 3.14159265358979323846f * 2.0f) currentRotation -= 3.14159265358979323846f * 2.0f;
				
				if(currentRotation < 3.14159265358979323846f){
					pev->avelocity.y = -120.0f + pHealth;
					SetThink( &CDiscoDisconator::ThinkRotating );
				}else{
					pev->avelocity.y =  120.0f - pHealth;
					SetThink( &CDiscoDisconator::ThinkRotating );
				}
			}
			pev->nextthink = gpGlobals->time + 0.0f;
		}
	}
	
	void ThinkRotating(){
		float currentRotation = pev->angles.y / 180.0f * 3.14159265358979323846f;
		float pathLength = sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y);
		if(pathLength == 0.0f){
			pev->avelocity.y = 0.0f;
			
			SetThink( &CDiscoDisconator::ThinkBreak1R );
		}else{
			float targetRotation = 0.0f;
			if(g_disco_aim_cross.targetPos.y < 0.0f) targetRotation = acos(-g_disco_aim_cross.targetPos.x/pathLength) - 3.14159265358979323846f;
			else targetRotation = acos(g_disco_aim_cross.targetPos.x/pathLength);
			
			currentRotation -= targetRotation;
			while(currentRotation < 0.0f) currentRotation += 3.14159265358979323846f * 2.0f;
			while(currentRotation > 3.14159265358979323846f * 2.0f) currentRotation -= 3.14159265358979323846f * 2.0f;
			
			if(currentRotation < 3.14159265358979323846f){
				if(pev->avelocity.y > 0.0f){
					
					SetThink( &CDiscoDisconator::ThinkBreak1L );
				}
			}else{
				if(pev->avelocity.y < 0.0f){
					
					SetThink( &CDiscoDisconator::ThinkBreak1R );
				}
			}
		}
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void ThinkBreak1L(){
		pev->avelocity.y = 0.0f;
		SetThink( &CDiscoDisconator::ThinkThrowBombLeft1 );
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	
	void ThinkBreak1R(){
		pev->avelocity.y = 0.0f;
		SetThink( &CDiscoDisconator::ThinkThrowBombRight1 );
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	
	void ThinkThrowBombRight1(){
		pev->avelocity.y = 0.0f;
		SetAnim(DISCONATOR_BOMBR);
		
		SetThink( &CDiscoDisconator::ThinkThrowBombRight2 );
		pev->nextthink = gpGlobals->time + 0.13f;
	}
	
	void ThinkThrowBombRight2(){
		Vector fireballPos = pev->origin;
		fireballPos.z += 80.0f;
		fireballPos.x = sin(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		fireballPos.y = -cos(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		
		CBaseEntity* pEntity = CBaseEntity::Create("disco_fireball", fireballPos, Vector(0, 0, 0), true);
		
		float power = 0.0f;
		float pHealth = (*gloVariables2->GetCustomKeyValues())["$f_health_percentage"].fVal;
		if(pHealth > 90.0f){
			power = RANDOM_FLOAT( 0.0f, 1.0f );
		}else{
			power = (sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y) -100.0f) / 900.0f;
			if(power < 0.0f) power = 0.0f;
			if(power > 1.0f) power = 1.0f;
		}
		
		pEntity->pev->velocity.x = cos(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.y = sin(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.z = 140.0f + 140.0f * power;
		
		SetThink( &CDiscoDisconator::ThinkRotateChooseDirection );
		pev->nextthink = gpGlobals->time + 0.53f;
	}
	
	void ThinkThrowBombLeft1(){
		pev->avelocity.y = 0.0f;
		SetAnim(DISCONATOR_BOMBL);
		
		SetThink( &CDiscoDisconator::ThinkThrowBombLeft2 );
		pev->nextthink = gpGlobals->time + 0.13f;
	}
	
	void ThinkThrowBombLeft2(){
		Vector fireballPos = pev->origin;
		fireballPos.z += 80.0f;
		fireballPos.x = -sin(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		fireballPos.y = cos(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		
		CBaseEntity* pEntity = CBaseEntity::Create("disco_fireball", fireballPos, Vector(0, 0, 0), true);
		
		float power = 0.0f;
		float pHealth = (*gloVariables2->GetCustomKeyValues())["$f_health_percentage"].fVal;
		if(pHealth > 90.0f){
			power = RANDOM_FLOAT( 0.0f, 1.0f );
		}else{
			power = (sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y) -100.0f) / 900.0f;
			if(power < 0.0f) power = 0.0f;
			if(power > 1.0f) power = 1.0f;
		}
		
		pEntity->pev->velocity.x = cos(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.y = sin(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.z = 140.0f + 140.0f * power;
		
		SetThink( &CDiscoDisconator::ThinkRotateChooseDirection );
		pev->nextthink = gpGlobals->time + 0.53f;
	}
};

class CDiscoDisconator2 : public CBaseAnimating {
public:
	CBaseEntity* gloVariables2;
	CBaseEntity* aimCross;
	
	// if youre gonna use this in your script, make sure you dont try to access invalid animations -zode
	void SetAnim(int animIndex) {
		pev->sequence = animIndex;
		pev->frame = 0;
		ResetSequenceInfo();
	}
	
	void Spawn() {
		Precache();
		
		SET_MODEL(edict(), "models/cubemath/discoboss/disconator.mdl");
		UTIL_SetOrigin(pev, pev->origin);
		
		pev->targetname = MAKE_STRING("disconator");
		pev->framerate = 1.0f;
		
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;

		SetAnim(DISCONATOR_DIE);
		
		for( int i = 0; i < gpGlobals->maxEntities; ++i ) {
			CBaseEntity* pEntity = CBaseEntity::Instance( INDEXENT(i) );
			
			if( pEntity ){
				if(!strcmp(STRING(pEntity->pev->targetname), "variables")) {
					gloVariables2 = pEntity;
				}
				if(!strcmp(STRING(pEntity->pev->targetname), "phase_2_breakable")) {
					pEntity->pev->rendermode = 0;
				}
				if(!strcmp(STRING(pEntity->pev->targetname), "disco_aim_cross")) {
					aimCross = pEntity;
				}
			}
		}
		
		SetThink( &CDiscoDisconator2::ThinkRotateChooseDirection );
		pev->nextthink = gpGlobals->time + 1.75f;
	}
	
	void ThinkRotateChooseDirection(){
		g_disco_aim_cross.ShufflePlayer();
		SetAnim(DISCONATOR_IDLE);
		
		float pHealth = (*gloVariables2->GetCustomKeyValues())["$f_health_percentage"].fVal;
		
		float currentRotation = pev->angles.y / 180.0f * 3.14159265358979323846f;
		float pathLength = sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y);
		if(pathLength == 0.0f){
			ThinkThrowBombRight1();
		}else{
			float targetRotation = 0.0f;
			if(g_disco_aim_cross.targetPos.y < 0.0f) targetRotation = acos(-g_disco_aim_cross.targetPos.x/pathLength) - 3.14159265358979323846f;
			else targetRotation = acos(g_disco_aim_cross.targetPos.x/pathLength);
			
			currentRotation -= targetRotation;
			while(currentRotation < 0.0f) currentRotation += 3.14159265358979323846f * 2.0f;
			while(currentRotation > 3.14159265358979323846f * 2.0f) currentRotation -= 3.14159265358979323846f * 2.0f;
			
			if(currentRotation < 3.14159265358979323846f){
				pev->avelocity.y = -120.0f + pHealth;
				SetThink( &CDiscoDisconator2::ThinkRotating );
			}else{
				pev->avelocity.y =  120.0f - pHealth;
				SetThink( &CDiscoDisconator2::ThinkRotating );
			}
		}
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void ThinkRotating(){
		float currentRotation = pev->angles.y / 180.0f * 3.14159265358979323846f;
		float pathLength = sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y);
		if(pathLength == 0.0f){
			pev->avelocity.y = 0.0f;
			
			SetThink( &CDiscoDisconator2::ThinkBreak2R );
		}else{
			float targetRotation = 0.0f;
			if(g_disco_aim_cross.targetPos.y < 0.0f) targetRotation = acos(-g_disco_aim_cross.targetPos.x/pathLength) - 3.14159265358979323846f;
			else targetRotation = acos(g_disco_aim_cross.targetPos.x/pathLength);
			
			currentRotation -= targetRotation;
			while(currentRotation < 0.0f) currentRotation += 3.14159265358979323846f * 2.0f;
			while(currentRotation > 3.14159265358979323846f * 2.0f) currentRotation -= 3.14159265358979323846f * 2.0f;
			
			if(currentRotation < 3.14159265358979323846f){
				if(pev->avelocity.y > 0.0f){
					
					SetThink( &CDiscoDisconator2::ThinkBreak2L );
				}
			}else{
				if(pev->avelocity.y < 0.0f){
					
					SetThink( &CDiscoDisconator2::ThinkBreak2R );
				}
			}
		}
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void ThinkBreak2L(){
		pev->avelocity.y = 0.0f;
		SetThink( &CDiscoDisconator2::ThinkThrowBombLeft1 );
		pev->nextthink = gpGlobals->time + 0.5f;
	}
	
	void ThinkBreak2R(){
		pev->avelocity.y = 0.0f;
		SetThink( &CDiscoDisconator2::ThinkThrowBombRight1 );
		pev->nextthink = gpGlobals->time + 0.5f;
	}
	
	void ThinkThrowBombRight1(){
		pev->avelocity.y = 0.0f;
		SetAnim(DISCONATOR_BOMBR);
		
		SetThink( &CDiscoDisconator2::ThinkThrowBombRight2 );
		pev->nextthink = gpGlobals->time + 0.13f;
	}
	
	void ThinkThrowBombRight2(){
		Vector fireballPos = pev->origin;
		fireballPos.z += 80.0f;
		fireballPos.x = sin(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		fireballPos.y = -cos(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		
		CBaseEntity* pEntity = CBaseEntity::Create("disco_fireball", fireballPos, Vector(0, 0, 0), true);
		
		float power = (sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y) -100.0f) / 900.0f;
		if(power < 0.0f) power = 0.0f;
		if(power > 1.0f) power = 1.0f;
		
		pEntity->pev->velocity.x = cos(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.y = sin(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.z = 140.0f + 140.0f * power;
		
		SetThink( &CDiscoDisconator2::ThinkRotateChooseDirection );
		pev->nextthink = gpGlobals->time + 0.53f;
	}
	
	void ThinkThrowBombLeft1(){
		pev->avelocity.y = 0.0f;
		SetAnim(DISCONATOR_BOMBL);
		
		SetThink( &CDiscoDisconator2::ThinkThrowBombLeft2 );
		pev->nextthink = gpGlobals->time + 0.13f;
	}
	
	void ThinkThrowBombLeft2(){
		Vector fireballPos = pev->origin;
		fireballPos.z += 80.0f;
		fireballPos.x = -sin(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		fireballPos.y = cos(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		
		CBaseEntity* pEntity = CBaseEntity::Create("disco_fireball", fireballPos, Vector(0, 0, 0), true);
		
		float power = (sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y) -100.0f) / 900.0f;
		if(power < 0.0f) power = 0.0f;
		if(power > 1.0f) power = 1.0f;
		
		pEntity->pev->velocity.x = cos(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.y = sin(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.z = 140.0f + 140.0f * power;
		
		SetThink( &CDiscoDisconator2::ThinkRotateChooseDirection );
		pev->nextthink = gpGlobals->time + 0.53f;
	}
};

class CDiscoDisconator3 : public CBaseAnimating {
public:
	CBaseEntity* gloVariables2;
	CBaseEntity* aimCross;
	
	// if youre gonna use this in your script, make sure you dont try to access invalid animations -zode
	void SetAnim(int animIndex) {
		pev->sequence = animIndex;
		pev->frame = 0;
		ResetSequenceInfo();
	}
	
	void Spawn() {
		Precache();
		
		SET_MODEL(edict(), "models/cubemath/discoboss/disconator.mdl");
		UTIL_SetOrigin(pev, pev->origin);
		
		pev->targetname = MAKE_STRING("disconator");
		pev->framerate = 1.0f;
		
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;

		SetAnim(DISCONATOR_DIE);
		
		for( int i = 0; i < gpGlobals->maxEntities; ++i ) {
			CBaseEntity* pEntity = CBaseEntity::Instance( INDEXENT(i) );
			
			if( pEntity ){
				if( !strcmp(STRING(pEntity->pev->targetname), "variables") ) {
					gloVariables2 = pEntity;
				}
				if(!strcmp(STRING(pEntity->pev->targetname), "disco_aim_cross")) {
					aimCross = pEntity;
				}
			}
		}
		
		SetThink( &CDiscoDisconator3::ThinkRotateChooseDirection );
		pev->nextthink = gpGlobals->time + 1.75f;
	}
	
	void ThinkRotateChooseDirection(){
		g_disco_aim_cross.ShufflePlayer();
		SetAnim(DISCONATOR_IDLE);
		
		float pHealth = (*gloVariables2->GetCustomKeyValues())["$f_health_percentage"].fVal;
		
		float currentRotation = pev->angles.y / 180.0f * 3.14159265358979323846f;
		float pathLength = sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y);
		if(pathLength == 0.0f){
			ThinkThrowBombRight1();
		}else{
			float targetRotation = 0.0f;
			if(g_disco_aim_cross.targetPos.y < 0.0f) targetRotation = acos(-g_disco_aim_cross.targetPos.x/pathLength) - 3.14159265358979323846f;
			else targetRotation = acos(g_disco_aim_cross.targetPos.x/pathLength);
			
			currentRotation -= targetRotation;
			while(currentRotation < 0.0f) currentRotation += 3.14159265358979323846f * 2.0f;
			while(currentRotation > 3.14159265358979323846f * 2.0f) currentRotation -= 3.14159265358979323846f * 2.0f;
			
			if(currentRotation < 3.14159265358979323846f){
				pev->avelocity.y = -120.0f + pHealth;
				SetThink( &CDiscoDisconator3::ThinkRotating );
			}else{
				pev->avelocity.y =  120.0f - pHealth;
				SetThink( &CDiscoDisconator3::ThinkRotating );
			}
		}
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void ThinkRotating(){
		float currentRotation = pev->angles.y / 180.0f * 3.14159265358979323846f;
		float pathLength = sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y);
		if(pathLength == 0.0f){
			pev->avelocity.y = 0.0f;
			
			SetThink( &CDiscoDisconator3::ThinkThrowBombRight1 );
		}else{
			float targetRotation = 0.0f;
			if(g_disco_aim_cross.targetPos.y < 0.0f) targetRotation = acos(-g_disco_aim_cross.targetPos.x/pathLength) - 3.14159265358979323846f;
			else targetRotation = acos(g_disco_aim_cross.targetPos.x/pathLength);
			
			currentRotation -= targetRotation;
			while(currentRotation < 0.0f) currentRotation += 3.14159265358979323846f * 2.0f;
			while(currentRotation > 3.14159265358979323846f * 2.0f) currentRotation -= 3.14159265358979323846f * 2.0f;
			
			if(currentRotation < 3.14159265358979323846f){
				if(pev->avelocity.y > 0.0f){
					
					SetThink( &CDiscoDisconator3::ThinkThrowBombLeft1 );
				}
			}else{
				if(pev->avelocity.y < 0.0f){
					
					SetThink( &CDiscoDisconator3::ThinkThrowBombRight1 );
				}
			}
		}
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void ThinkThrowBombRight1(){
		pev->avelocity.y = 0.0f;
		SetAnim(DISCONATOR_BOMBR);
		
		SetThink( &CDiscoDisconator3::ThinkThrowBombRight2 );
		pev->nextthink = gpGlobals->time + 0.13f;
	}
	
	void ThinkThrowBombRight2(){
		Vector fireballPos = pev->origin;
		fireballPos.z += 80.0f;
		fireballPos.x = sin(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		fireballPos.y = -cos(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		
		CBaseEntity* pEntity = CBaseEntity::Create("disco_fireball", fireballPos, Vector(0, 0, 0), true);
		
		float power = (sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y) -100.0f) / 900.0f;
		if(power < 0.0f) power = 0.0f;
		if(power > 1.0f) power = 1.0f;
		
		pEntity->pev->velocity.x = cos(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.y = sin(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.z = 140.0f + 140.0f * power;
		
		SetThink( &CDiscoDisconator3::ThinkRotateChooseDirection );
		pev->nextthink = gpGlobals->time + 0.53f;
	}
	
	void ThinkThrowBombLeft1(){
		pev->avelocity.y = 0.0f;
		SetAnim(DISCONATOR_BOMBL);
		
		SetThink( &CDiscoDisconator3::ThinkThrowBombLeft2 );
		pev->nextthink = gpGlobals->time + 0.13f;
	}
	
	void ThinkThrowBombLeft2(){
		Vector fireballPos = pev->origin;
		fireballPos.z += 80.0f;
		fireballPos.x = -sin(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		fireballPos.y = cos(pev->angles.y/180.0f*3.14159265358979323846f) * 16.0f;
		
		CBaseEntity* pEntity = CBaseEntity::Create("disco_fireball", fireballPos, Vector(0, 0, 0), true);
		
		float power = (sqrt(g_disco_aim_cross.targetPos.x*g_disco_aim_cross.targetPos.x+g_disco_aim_cross.targetPos.y*g_disco_aim_cross.targetPos.y) -100.0f) / 900.0f;
		if(power < 0.0f) power = 0.0f;
		if(power > 1.0f) power = 1.0f;
		
		pEntity->pev->velocity.x = cos(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.y = sin(pev->angles.y/180.0f*3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.z = 140.0f + 140.0f * power;
		
		SetThink( &CDiscoDisconator3::ThinkRotateChooseDirection );
		pev->nextthink = gpGlobals->time + 0.53f;
	}
};

class CDiscoDisconator4 : public CBaseAnimating {
public:

	// if youre gonna use this in your script, make sure you dont try to access invalid animations -zode
	void SetAnim(int animIndex) {
		pev->sequence = animIndex;
		pev->frame = 0;
		ResetSequenceInfo();
	}

	void Spawn() {
		Precache();

		SET_MODEL(edict(), "models/cubemath/discoboss/disconatorbroken.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->targetname = MAKE_STRING("disconator");
		pev->framerate = 1.0f;

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;

		SetAnim(DISCONATOR_DIE);

		SetThink(&CDiscoDisconator4::ThinkPreThrowBomb);
		pev->nextthink = gpGlobals->time + 2.5f;
	}

	void ThinkPreThrowBomb() {
		SetAnim(DISCONATOR_RAGE);
		ThinkThrowBombLeft2();
	}

	void ThinkThrowBombRight2() {
		pev->frame = 8;
		pev->avelocity.y = RANDOM_FLOAT(90.0f, 120.0f);

		Vector fireballPos = pev->origin;
		fireballPos.z += 80.0f;
		fireballPos.x = sin(pev->angles.y / 180.0f * 3.14159265358979323846f) * 16.0f;
		fireballPos.y = -cos(pev->angles.y / 180.0f * 3.14159265358979323846f) * 16.0f;

		CBaseEntity* pEntity = CBaseEntity::Create("disco_fireball", fireballPos, Vector(0, 0, 0), true);

		float power = RANDOM_FLOAT(0.0f, 1.0f);

		pEntity->pev->velocity.x = sin(pev->angles.y / 180.0f * 3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.y = -cos(pev->angles.y / 180.0f * 3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.z = 140.0f + 140.0f * power;

		SetThink(&CDiscoDisconator4::ThinkThrowBombLeft2);
		pev->nextthink = gpGlobals->time + 0.53f;
	}

	void ThinkThrowBombLeft2() {
		pev->frame = 0;
		pev->avelocity.y = RANDOM_FLOAT(90.0f, 120.0f);

		Vector fireballPos = pev->origin;
		fireballPos.z += 80.0f;
		fireballPos.x = -sin(pev->angles.y / 180.0f * 3.14159265358979323846f) * 16.0f;
		fireballPos.y = cos(pev->angles.y / 180.0f * 3.14159265358979323846f) * 16.0f;

		CBaseEntity* pEntity = CBaseEntity::Create("disco_fireball", fireballPos, Vector(0, 0, 0), true);

		float power = RANDOM_FLOAT(0.0f, 1.0f);

		pEntity->pev->velocity.x = -sin(pev->angles.y / 180.0f * 3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.y = cos(pev->angles.y / 180.0f * 3.14159265358979323846f) * (60.0f + 490.0f * power);
		pEntity->pev->velocity.z = 140.0f + 140.0f * power;

		SetThink(&CDiscoDisconator4::ThinkThrowBombRight2);
		pev->nextthink = gpGlobals->time + 0.53f;
	}
};

class CDiscoDisconatorDying : public CBaseAnimating {
public:
	void Spawn() {
		Precache();

		SET_MODEL(edict(), "models/cubemath/discoboss/disconatorbroken.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->targetname = MAKE_STRING("disconator");
		pev->framerate = 1.0f;

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;

		pev->sequence = DISCONATOR_DIE;
		pev->frame = 0;
		ResetSequenceInfo();

		pev->nextthink = gpGlobals->time + 5.0f;
		SetThink(&CDiscoDisconatorDying::DieThink);
	}

	void DieThink() {
		UTIL_Remove(this);
	}
};

LINK_ENTITY_TO_CLASS(disco_health_logic, CDiscoHealthLogic)
LINK_ENTITY_TO_CLASS(disco_disconator, CDiscoDisconator)
LINK_ENTITY_TO_CLASS(disco_disconator2, CDiscoDisconator2)
LINK_ENTITY_TO_CLASS(disco_disconator3, CDiscoDisconator3)
LINK_ENTITY_TO_CLASS(disco_disconator4, CDiscoDisconator4)
LINK_ENTITY_TO_CLASS(disco_disconator_dying, CDiscoDisconatorDying)
LINK_ENTITY_TO_CLASS(disco_fireball, CDiscoFireball)

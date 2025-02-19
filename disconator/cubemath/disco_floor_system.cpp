#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "hlds_hooks.h"

using namespace std;

extern float g_musicBeginTime;

class CDiscoFloorSystem : public CBaseEntity {
	
	vector<vector<CBaseEntity*>> disco_floor;
	vector<vector<CBaseEntity*>> disco_hurt;
	CBaseEntity* gloVariables;
	
	//0 = Not Hurting (green)
	//1 = Next one will hurt (yellow)
	//2 = Hurting (red)
	//3 = Hurt ends soon (red)
	vector<vector<int>> disco_hurt_status;
	
	float phaseBeginTime;
	int soundStep;
	
	void Spawn() {
		Precache();
		
		SetThink( &CDiscoFloorSystem::PrepareThink );
		pev->nextthink = gpGlobals->time + 1.0f;
		
		//Randomness of Tiles:
		//0.0f = Everything Green
		//100.0f = Everything Red
		pev->dmg = 5.0f;
		
		soundStep = 0;
		phaseBeginTime = 0.0f;
	}
	
	void PrepareThink() {
		disco_floor.resize( 16 );
		disco_hurt.resize( 16 );
		disco_hurt_status.resize( 16 );
		for( int i = 0; i < 16; ++i ){
			disco_floor[i].resize( 16 );
			disco_hurt[i].resize( 16 );
			disco_hurt_status[i].resize( 16 );
			for( int y = 0; y < 16; ++y ){
				disco_hurt_status[i][y] = 0;
			}
		}
		
		for( int i = 0; i < gpGlobals->maxEntities; ++i ) {
			CBaseEntity* pEntity = CBaseEntity::Instance( INDEXENT(i) );
			if(pEntity){
				string targetname = STRING(pEntity->pev->targetname);
				
				if( targetname.find("disco_floor_") == 0 ) {
					int x = targetname[12]-48;
					int y = 0;
					if(targetname[13] > 57){
						y = targetname[14]-48;
						if(targetname.length() > 15) {
							y = y*10 + targetname[15]-48;
						}
					}else{
						x = x*10 + targetname[13]-48;
						y = targetname[15]-48;
						if(targetname.length() > 16) {
							y = y*10 + targetname[16]-48;
						}
					}
					
					disco_floor[x][y] = pEntity;
				}else if( targetname.find("disco_hurt_") == 0 ) {
					if(targetname[11] > 57 || targetname[11] < 48) continue;
					
					int x = targetname[11]-48;
					int y = 0;
					if(targetname[12] > 57){
						y = targetname[13]-48;
						if(targetname.length() > 14) {
							y = y*10 + targetname[14]-48;
						}
					}else{
						x = x*10 + targetname[12]-48;
						y = targetname[14]-48;
						if(targetname.length() > 15) {
							y = y*10 + targetname[15]-48;
						}
					}
					
					disco_hurt[x][y] = pEntity;
				}else if( targetname == "variables") {
					gloVariables = pEntity;
				}
			}
		}
		
		SetThink( &CDiscoFloorSystem::Think01 );
		pev->nextthink = gpGlobals->time + 0.1f;
	}
	
	void Think01() {
		float timer2 = fmodf(gpGlobals->time, 4.0f);
		for( int x = 0; x < 16; ++x ){
			for( int y = 0; y < 16; ++y ){
			
				if( disco_floor[x][y]  == NULL ) continue;
				
				float timer = fmodf(64.0f + timer2 - sqrt((x-7.5f)*(x-7.5f)+(y-7.5f)*(y-7.5f)) *0.5f, 4.0f);
				
				float g = 0;
				float b = 0;
				
				if(timer < 1.0f){
					g = 255.0f;
					b = timer * 255.0f;
				}else if(timer < 2.0f){
					g = 255.0f - (timer-1.0f) * 255.0f;
					b = 255.0f;
				}else if(timer < 3.0f){
					g = (timer-2.0f) * 255.0f;
					b = 255.0f;
				}else{
					g = 255.0f;
					b = 255.0f - (timer-3.0f) * 255.0f;
				}
				
				string colStr = "0 "+to_string(g)+" "+to_string(b);
				DispatchKeyValue( disco_floor[x][y]->edict(), "rendercolor", colStr.c_str() );
			}
		}
		
		float healthPercentage = gloVariables->GetCustomKeyValue("$f_health_percentage").fVal;
		if( healthPercentage <= 75.0f && healthPercentage > 0.0f ){
			phaseBeginTime = gpGlobals->time;
			SetThink( &CDiscoFloorSystem::Think02 );
		}
		
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void Think02() {
		float timer2 = fmodf(gpGlobals->time, 4.0f);
		float multiplicator = ((3.1378540146f * 2.49f + 1.1f) - (gpGlobals->time - phaseBeginTime))/(3.1378540146f * 2.49f + 1.1f);
		if(multiplicator < 0.0f) {
			multiplicator = 0.0f;
			phaseBeginTime += 3.1378540146f * 1.5f + 1.1f;
			SetThink( &CDiscoFloorSystem::Think03 );
		}
		
		for( int x = 0; x < 16; ++x ){
			for( int y = 0; y < 16; ++y ){
			
				if( disco_floor[x][y]  == NULL ) continue;
				
				float timer = fmodf(64.0f + timer2 - sqrt((x-7.5f)*(x-7.5f)+(y-7.5f)*(y-7.5f)) *0.5f, 4.0f);
				
				float g = 0;
				float b = 0;
				
				if(timer < 2.0f){
					timer = timer * multiplicator;
				}else{
					timer = 4.0 - (4.0 - timer) * multiplicator;
				}
				
				if(timer < 1.0f){
					g = 255.0f;
					b = timer * 255.0f;
				}else if(timer < 2.0f){
					g = 255.0f - (timer-1.0f) * 255.0f;
					b = 255.0f;
				}else if(timer < 3.0f){
					g = (timer-2.0f) * 255.0f;
					b = 255.0f;
				}else{
					g = 255.0f;
					b = 255.0f - (timer-3.0f) * 255.0f;
				}
				
				string colStr = "0 "+to_string(g)+" "+to_string(b);
				DispatchKeyValue( disco_floor[x][y]->edict(), "rendercolor", colStr.c_str() );
			}
		}
		
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void Think03() {
		float stepTime = (gpGlobals->time - phaseBeginTime)/3.1378540146f;
		if(stepTime > 1.0f) {
			phaseBeginTime += 3.1378540146f;
			stepTime -= 1.0f;
			
			//1 will turn to 2
			//3 will turn to 0
			for( int x = 0; x < 16; ++x ){
				for( int y = 0; y < 16; ++y ){
					if(disco_hurt_status[x][y] == 1){
						disco_hurt_status[x][y] = 2;
					}
					if(disco_hurt_status[x][y] == 3){
						disco_hurt_status[x][y] = 0;
					}
				}
			}
			
			//Random-Adder
			for( int x = 0; x < 16; ++x ){
				for( int y = 0; y < 16; ++y ){
					if(RANDOM_FLOAT( 0.0f, 100.0f ) > pev->dmg ){
						if(disco_hurt_status[x][y] == 2) ++disco_hurt_status[x][y];
					}else{
						if(disco_hurt_status[x][y] == 0) ++disco_hurt_status[x][y];
					}
				}
			}
			
			float healthPercentage = gloVariables->GetCustomKeyValue("$f_health_percentage").fVal;
			pev->dmg = 15.0f - (healthPercentage - 50.0f) * 0.4f;
			if( healthPercentage <= 50.0f ){
				phaseBeginTime = g_musicBeginTime + 1.91997959184f * 1.5f;
				SetThink( &CDiscoFloorSystem::Think04 );
			}
		}
		
		//RenderFloor
		for( int x = 0; x < 16; ++x ){
			for( int y = 0; y < 16; ++y ){
				if(disco_floor[x][y]  == NULL ) continue;
				
				float r = 0;
				float g = 0;
				
				if(disco_hurt_status[x][y] == 0){
					g = 255.0f;
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_disabled" );
				}else if(disco_hurt_status[x][y] == 1){
					g = 255.0f;
					r = 255.0f;
					if(stepTime >= 0.625f){
						g -= 64.0f-fmodf(stepTime,0.125f)*512.0f;
					}
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_disabled" );
				}else if(disco_hurt_status[x][y] == 2){
					r = 255.0f;
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_enabled" );
				}else{
					r = 255.0f;
					if(stepTime >= 0.625f){
						g = 64.0f- fmodf(stepTime,0.125f)*512.0f;
					}
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_enabled" );
				}
				
				string colStr = ""+to_string(r)+" "+to_string(g)+" 0";
				DispatchKeyValue( disco_floor[x][y]->edict(), "rendercolor", colStr.c_str() );
			}
		}
		
		if((stepTime+0.04f) >= 0.625f){
			if(fmodf(stepTime+0.04f, 0.125f) < 0.0625f){
				if(soundStep == 0){
					if((stepTime+0.02f) >= 0.875f){
						FireTargets("floor_changing_sfx", NULL, NULL, USE_ON, 0.0f);
					}else{
						FireTargets("floor_beeping_sfx", NULL, NULL, USE_ON, 0.0f);
					}
					soundStep = 1;
				}
			}else{
				soundStep = 0;
			}
		}else{
			soundStep = 0;
		}
		
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void Think04() {
		float stepTime = (gpGlobals->time - phaseBeginTime)/1.91997959184f;
		if(stepTime > 1.0f) {
			phaseBeginTime += 1.91997959184f;
			stepTime -= 1.0f;
			
			//1 will turn to 2
			//3 will turn to 0
			for( int x = 0; x < 16; ++x ){
				for( int y = 0; y < 16; ++y ){
					if(disco_hurt_status[x][y] == 1){
						disco_hurt_status[x][y] = 2;
					}
					if(disco_hurt_status[x][y] == 3){
						disco_hurt_status[x][y] = 0;
					}
				}
			}
			
			//Random-Adder
			for( int x = 0; x < 16; ++x ){
				for( int y = 0; y < 16; ++y ){
					if(RANDOM_FLOAT( 0.0f, 100.0f ) > pev->dmg ){
						if(disco_hurt_status[x][y] == 2) ++disco_hurt_status[x][y];
					}else{
						if(disco_hurt_status[x][y] == 0) ++disco_hurt_status[x][y];
					}
				}
			}
			
			float healthPercentage = gloVariables->GetCustomKeyValue("$f_health_percentage").fVal;
			pev->dmg = 25.0f - (healthPercentage - 25.0f) * 0.4f;
			if( healthPercentage <= 25.0f ){
				phaseBeginTime = g_musicBeginTime + 1.69019608f;
				SetThink( &CDiscoFloorSystem::Think05 );
				pev->dmg = 25.0f;
			}
		}
		
		//RenderFloor
		for( int x = 0; x < 16; ++x ){
			for( int y = 0; y < 16; ++y ){
				if(disco_floor[x][y]  == NULL ) continue;
				
				float r = 0;
				float g = 0;
				
				if(disco_hurt_status[x][y] == 0){
					g = 255.0f;
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_disabled" );
				}else if(disco_hurt_status[x][y] == 1){
					g = 255.0f;
					r = 255.0f;
					if(stepTime >= 0.5f){
						g -= 64.0f-fmodf(stepTime, 0.25f)*256.0f;
					}
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_disabled" );
				}else if(disco_hurt_status[x][y] == 2){
					r = 255.0f;
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_enabled" );
				}else{
					r = 255.0f;
					if(stepTime >= 0.5f){
						g = 64.0f- fmodf(stepTime, 0.25f)*256.0f;
					}
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_enabled" );
				}
				
				string colStr = ""+to_string(r)+" "+to_string(g)+" 0";
				DispatchKeyValue( disco_floor[x][y]->edict(), "rendercolor", colStr.c_str() );
			}
		}
		
		if((stepTime+0.04f) >= 0.5f){
			if(fmodf(stepTime+0.04f,0.25f) < 0.125f){
				if(soundStep == 0){
					if((stepTime+0.02f) >= 0.75f){
						FireTargets("floor_changing_sfx", NULL, NULL, USE_ON, 0.0f);
					}else{
						FireTargets("floor_beeping_sfx", NULL, NULL, USE_ON, 0.0f);
					}
					soundStep = 1;
				}
			}else{
				soundStep = 0;
			}
		}else{
			soundStep = 0;
		}
		
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void Think05() {
		
		float stepTime = (gpGlobals->time - phaseBeginTime)/1.69019608f;
		if(stepTime > 1.0f) {
			phaseBeginTime += 1.69019608f;
			stepTime -= 1.0f;
			
			//1 will turn to 2
			//3 will turn to 0
			for( int x = 0; x < 16; ++x ){
				for( int y = 0; y < 16; ++y ){
					if(disco_hurt_status[x][y] == 1){
						disco_hurt_status[x][y] = 2;
					}
					if(disco_hurt_status[x][y] == 3){
						disco_hurt_status[x][y] = 0;
					}
				}
			}
			
			//Random-Adder
			for( int x = 0; x < 16; ++x ){
				for( int y = 0; y < 16; ++y ){
					if(RANDOM_FLOAT( 0.0f, 100.0f ) > pev->dmg ){
						if(disco_hurt_status[x][y] == 2) ++disco_hurt_status[x][y];
					}else{
						if(disco_hurt_status[x][y] == 0) ++disco_hurt_status[x][y];
					}
				}
			}
		}
		
		//RenderFloor
		for( int x = 0; x < 16; ++x ){
			for( int y = 0; y < 16; ++y ){
				if(disco_floor[x][y]  == NULL ) continue;
				
				float r = 0;
				float g = 0;
				
				if(disco_hurt_status[x][y] == 0){
					g = 255.0f;
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_disabled" );
				}else if(disco_hurt_status[x][y] == 1){
					g = 255.0f;
					r = 255.0f;
					if(stepTime >= 0.5f){
						g -= 64.0f- fmodf(stepTime,0.25f)*256.0f;
					}
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_disabled" );
				}else if(disco_hurt_status[x][y] == 2){
					r = 255.0f;
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_enabled" );
				}else{
					r = 255.0f;
					if(stepTime >= 0.5f){
						g = 64.0f- fmodf(stepTime, 0.25f)*256.0f;
					}
					DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_enabled" );
				}
				
				string colStr = ""+to_string(r)+" "+to_string(g)+" 0";
				DispatchKeyValue( disco_floor[x][y]->edict(), "rendercolor", colStr.c_str() );
			}
		}
		
		float healthPercentage2 = gloVariables->GetCustomKeyValue("$f_health_percentage").fVal;
		if( healthPercentage2 <= 0.0f ){
			SetThink( &CDiscoFloorSystem::Think06 );
		}
		
		if(stepTime+0.04f >= 0.5f){
			if(fmodf(stepTime+0.04f, 0.25f) < 0.125f){
				if(soundStep == 0){
					if((stepTime+0.02f) >= 0.75f){
						FireTargets("floor_changing_sfx", NULL, NULL, USE_ON, 0.0f);
					}else{
						FireTargets("floor_beeping_sfx", NULL, NULL, USE_ON, 0.0f);
					}
					soundStep = 1;
				}
			}else{
				soundStep = 0;
			}
		}else{
			soundStep = 0;
		}
		
		pev->nextthink = gpGlobals->time + 0.0f;
	}
	
	void Think06() {
		for( int x = 0; x < 16; ++x ){
			for( int y = 0; y < 16; ++y ){
				if(disco_floor[x][y]  == NULL ) continue;
				DispatchKeyValue( disco_hurt[x][y]->edict(), "master", "disco_hurt_disabled" );
			}
		}
		
		SetThink( &CDiscoFloorSystem::Think01 );
		pev->nextthink = gpGlobals->time + 0.0f;
	}
};

LINK_ENTITY_TO_CLASS(disco_floor_system, CDiscoFloorSystem)

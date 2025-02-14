#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"

using namespace std;

extern int isBossRunning;

class CDiscoDrone : public CBaseEntity {
public:
	vector<EHANDLE> grenades;
	int currentArrIdx;
	int numberPeople;
	
	void Spawn() {
		Precache();
		
		pev->speed = 0;
		
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;

		SET_MODEL(edict(), STRING(pev->model));
		UTIL_SetSize(pev, pev->mins, pev->maxs);
		UTIL_SetOrigin(pev, pev->origin);
		
		currentArrIdx = 0;
		
		grenades.resize( 32 );
		
		SetThink( &CDiscoDrone::WaitThink );
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	
	void WaitThink(){
		if(isBossRunning == 1){
			SetThink( &CDiscoDrone::FlyToTargetThink );
		}
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	
	//TODO Need better Calculation
	void FlyToTargetThink() {
		
		numberPeople = 0;
		int lowestGrenadeAmount = 10;
		int lowestGrenadeCount = -1;
		
		CBasePlayer* pPlayer = NULL;
		for( int iPlayer = 1; iPlayer <= gpGlobals->maxClients; ++iPlayer ){
			pPlayer = UTIL_PlayerByIndex( iPlayer );
			
			if( pPlayer == NULL || !pPlayer->IsAlive() )
				continue;
			
			numberPeople += 1;
			if(lowestGrenadeAmount > pPlayer->AmmoInventory(pPlayer->GetAmmoIndex("Hand Grenade"))){
				lowestGrenadeAmount = pPlayer->AmmoInventory(pPlayer->GetAmmoIndex("Hand Grenade"));
				lowestGrenadeCount = 0;
			}else if(lowestGrenadeAmount == pPlayer->AmmoInventory(pPlayer->GetAmmoIndex("Hand Grenade"))){
				lowestGrenadeCount++;
			}
		}
		
		float x1 = 0.0f;
		float y1 = 0.0f;
		
		if(lowestGrenadeCount > 0 ){
			int targetGrenadeCount = RANDOM_LONG( 0, lowestGrenadeCount );
			
			lowestGrenadeCount = 0;
			
			for( int iPlayer = 1; iPlayer <= gpGlobals->maxClients; ++iPlayer ){
				pPlayer = UTIL_PlayerByIndex( iPlayer );
				
				if( pPlayer  == NULL || !pPlayer->IsAlive() )
					continue;
				
				if(lowestGrenadeAmount == pPlayer->AmmoInventory(pPlayer->GetAmmoIndex("Hand Grenade"))){
					lowestGrenadeCount++;
					
					if(lowestGrenadeCount > targetGrenadeCount){
						x1 = pPlayer->pev->origin.x;
						y1 = pPlayer->pev->origin.y;
						break;
					}
				}
			}
		}
		
		if(x1 == 0.0f && y1 == 0.0f){
			lowestGrenadeAmount = 10;
		}
		
		float circleSize = sqrt(RANDOM_FLOAT( 0.0f, 1.0f )) * 900.0f;
		float circleRoti = RANDOM_FLOAT( -3.14159265358979323846f, 3.14159265358979323846f );
		float x2 = sin(circleRoti) * circleSize;
		float y2 = cos(circleRoti) * circleSize;
		
		float speed = 64.0f + 32.0f * numberPeople;
		float x = (float(lowestGrenadeAmount) * x2 + float(10-lowestGrenadeAmount) * x1)/10.0f;
		float y = (float(lowestGrenadeAmount) * y2 + float(10-lowestGrenadeAmount) * y1)/10.0f;
		float distance = sqrt( (x-pev->origin.x)*(x-pev->origin.x) + (y-pev->origin.y)*(y-pev->origin.y) );
		
		if(distance < 0.01f) distance = 0.01f;
		
		pev->velocity.x = ((x-pev->origin.x)/distance)*speed;
		pev->velocity.y = ((y-pev->origin.y)/distance)*speed;
		
		SetThink( &CDiscoDrone::DropgrenadeThink1 );
		pev->nextthink = gpGlobals->time + (distance/speed);
	}
	
	void DropgrenadeThink1() {
		pev->velocity.x = 0.0f;
		pev->velocity.y = 0.0f;
		
		SetThink( &CDiscoDrone::DropgrenadeThink2 );
		
		int np = numberPeople;
		if(np > 16) np = 16;
		pev->nextthink = gpGlobals->time + 1.6f - 0.1f * float(np);
	}
	
	void DropgrenadeThink2() {
		int haveSpace = -1;
		
		currentArrIdx++;
		if(currentArrIdx >= 32){
			currentArrIdx = 0;
		}
		
		//Destroy Grenade-Ammo
		if( grenades[currentArrIdx] ){
			UTIL_Remove( grenades[currentArrIdx] );
		}
		
		//Create Grenade-Ammo
		grenades[currentArrIdx] = CBaseEntity::Create("weapon_handgrenade", pev->origin, Vector(0, 0, 0), true);
		
		SetThink( &CDiscoDrone::FlyToTargetThink );
		int np = numberPeople;
		if(np > 16) np = 16;
		pev->nextthink = gpGlobals->time + 1.6f - 0.1f * float(np);
	}
};

LINK_ENTITY_TO_CLASS(disco_drone, CDiscoDrone)

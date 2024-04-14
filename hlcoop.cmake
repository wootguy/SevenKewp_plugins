cmake_minimum_required(VERSION 3.2)

set(ROOT_FOLDER ${CMAKE_CURRENT_LIST_DIR})
set(INC_FOLDER ${ROOT_FOLDER}/include/hlcoop)

set(MISC_HDR
	${INC_FOLDER}/engine/studio.h
	${INC_FOLDER}/common/Platform.h
	${INC_FOLDER}/dlls/cdll_dll.h
	${INC_FOLDER}/dlls/enginecallback.h
	${INC_FOLDER}/dlls/extdll.h
)

set(MONSTER_HDR
	${INC_FOLDER}/dlls/monster/activity.h
	${INC_FOLDER}/dlls/monster/activitymap.h
	${INC_FOLDER}/dlls/monster/bodyque.h
	${INC_FOLDER}/dlls/monster/CAGrunt.h
	${INC_FOLDER}/dlls/monster/CBaseGrunt.h
	${INC_FOLDER}/dlls/monster/CBaseGruntOp4.h
	${INC_FOLDER}/dlls/monster/CBaseMonster.h
	${INC_FOLDER}/dlls/monster/CBaseTurret.h
	${INC_FOLDER}/dlls/monster/CFlyingMonster.h
	${INC_FOLDER}/dlls/monster/CGargantua.h
	${INC_FOLDER}/dlls/monster/CHornet.h
	${INC_FOLDER}/dlls/monster/CMonsterMaker.h
	${INC_FOLDER}/dlls/monster/CTalkSquadMonster.h
	${INC_FOLDER}/dlls/monster/defaultai.h
	${INC_FOLDER}/dlls/monster/monsterevent.h
	${INC_FOLDER}/dlls/monster/monsters.h
	${INC_FOLDER}/dlls/monster/schedule.h
)

set(ENV_HDR
	${INC_FOLDER}/dlls/env/CAmbientGeneric.h
	${INC_FOLDER}/dlls/env/CBeam.h
	${INC_FOLDER}/dlls/env/CBloodSplat.h
	${INC_FOLDER}/dlls/env/CGibShooter.h
	${INC_FOLDER}/dlls/env/CLaser.h
	${INC_FOLDER}/dlls/env/CLight.h
	${INC_FOLDER}/dlls/env/CSoundEnt.h
	${INC_FOLDER}/dlls/env/CSprayCan.h
	${INC_FOLDER}/dlls/env/CSprite.h
	${INC_FOLDER}/dlls/env/decals.h
	${INC_FOLDER}/dlls/env/effects.h
	${INC_FOLDER}/dlls/env/explode.h
	${INC_FOLDER}/engine/shake.h
)

set(ITEM_HDR
	${INC_FOLDER}/dlls/item/CItem.h
)

set(TRIGGER_HDR
	${INC_FOLDER}/dlls/triggers/CBaseLogic.h
	${INC_FOLDER}/dlls/triggers/CBaseTrigger.h
	${INC_FOLDER}/dlls/triggers/CGamePlayerEquip.h
	${INC_FOLDER}/dlls/triggers/CFireAndDie.h
	${INC_FOLDER}/dlls/triggers/CRuleEntity.h
	${INC_FOLDER}/dlls/triggers/CTriggerMultiple.h
)

set(FUNC_HDR
	${INC_FOLDER}/dlls/func/CBaseButton.h
	${INC_FOLDER}/dlls/func/CBasePlatTrain.h
	${INC_FOLDER}/dlls/func/CBreakable.h
	${INC_FOLDER}/dlls/func/CBaseDoor.h
	${INC_FOLDER}/dlls/func/CFuncPlat.h
	${INC_FOLDER}/dlls/func/CFuncPlatRot.h
	${INC_FOLDER}/dlls/func/CFuncTank.h
	${INC_FOLDER}/dlls/func/CFuncTrackChange.h
	${INC_FOLDER}/dlls/func/CFuncTrackTrain.h
	${INC_FOLDER}/dlls/func/CFuncWall.h
	${INC_FOLDER}/dlls/func/CPlatTrigger.h
)

set(ENTITY_HDR
	${INC_FOLDER}/dlls/cbase.h
	${INC_FOLDER}/dlls/ent_globals.h
	${INC_FOLDER}/dlls/nodes.h
	${INC_FOLDER}/dlls/CBasePlayer.h
	${INC_FOLDER}/dlls/CBaseSpectator.h
	${INC_FOLDER}/dlls/CKeyValue.h
	${INC_FOLDER}/dlls/scripted.h
	${INC_FOLDER}/dlls/scriptevent.h
	${INC_FOLDER}/dlls/path/CPathCorner.h
	${INC_FOLDER}/dlls/path/CPathTrack.h
	${INC_FOLDER}/engine/progs.h
	${INC_FOLDER}/engine/progdefs.h
	${INC_FOLDER}/engine/edict.h
	${INC_FOLDER}/engine/customentity.h
	${INC_FOLDER}/common/const.h
)

set(WEAPON_HDR
	${INC_FOLDER}/dlls/weapon/CShockBeam.h
	${INC_FOLDER}/dlls/weapon/CGrapple.h
	${INC_FOLDER}/dlls/weapon/CGrappleTip.h
	${INC_FOLDER}/dlls/weapon/CSpore.h
	${INC_FOLDER}/dlls/weapon/CBasePlayerItem.h
	${INC_FOLDER}/dlls/weapon/CBasePlayerWeapon.h
	${INC_FOLDER}/dlls/weapon/CWeaponBox.h
	${INC_FOLDER}/dlls/weapon/CGlock.h
	${INC_FOLDER}/dlls/weapon/CCrowbar.h
	${INC_FOLDER}/dlls/weapon/CPython.h
	${INC_FOLDER}/dlls/weapon/CMP5.h
	${INC_FOLDER}/dlls/weapon/CCrossbow.h
	${INC_FOLDER}/dlls/weapon/CShotgun.h
	${INC_FOLDER}/dlls/weapon/CRpg.h
	${INC_FOLDER}/dlls/weapon/CGauss.h
	${INC_FOLDER}/dlls/weapon/CEgon.h
	${INC_FOLDER}/dlls/weapon/CHgun.h
	${INC_FOLDER}/dlls/weapon/CHandGrenade.h
	${INC_FOLDER}/dlls/weapon/CSatchel.h
	${INC_FOLDER}/dlls/weapon/CTripmine.h
	${INC_FOLDER}/dlls/weapon/CSqueak.h
	${INC_FOLDER}/dlls/weapon/weapons.h
)

set(AMMO_HDR
	${INC_FOLDER}/dlls/weapon/CBasePlayerAmmo.h
	${INC_FOLDER}/dlls/weapon/ammo.h
)

set(UTIL_HDR
	${INC_FOLDER}/dlls/animation.h
	${INC_FOLDER}/dlls/Bsp.h
	${INC_FOLDER}/dlls/bsptypes.h
	${INC_FOLDER}/dlls/bsplimits.h
	${INC_FOLDER}/dlls/plane.h
	${INC_FOLDER}/dlls/util.h
	${INC_FOLDER}/dlls/mstream.h
	${INC_FOLDER}/dlls/lagcomp.h
	${INC_FOLDER}/dlls/vector.h
	${INC_FOLDER}/game_shared/shared_util.h
)

set(GAME_HDR
	${INC_FOLDER}/dlls/client.h
	${INC_FOLDER}/engine/custom.h
	${INC_FOLDER}/engine/eiface.h
	${INC_FOLDER}/dlls/game.h
	${INC_FOLDER}/dlls/gamerules.h
	${INC_FOLDER}/dlls/saverestore.h
	${INC_FOLDER}/dlls/skill.h
	${INC_FOLDER}/dlls/teamplay_gamerules.h
	${INC_FOLDER}/dlls/user_messages.h
	${INC_FOLDER}/dlls/mod_api.h
)

set(HLCOOP_HEADERS
	${MONSTER_HDR}
	${ENTITY_HDR}
	${ENV_HDR}
	${ITEM_HDR}
	${PATH_HDR}
	${WEAPON_HDR}
	${AMMO_HDR}
	${GAME_HDR}
	${UTIL_HDR}
	${MISC_HDR}
)

# setup standard include directories and compile settings
function(hlcoop_sdk_init)	
	include_directories(${ROOT_FOLDER}/include/hlcoop/common)
	include_directories(${ROOT_FOLDER}/include/hlcoop/engine)
	include_directories(${ROOT_FOLDER}/include/hlcoop/game_shared)
	include_directories(${ROOT_FOLDER}/include/hlcoop/pm_shared)
	include_directories(${ROOT_FOLDER}/include/hlcoop/public)
	include_directories(${ROOT_FOLDER}/include/hlcoop/dlls/env)
	include_directories(${ROOT_FOLDER}/include/hlcoop/dlls/func)
	include_directories(${ROOT_FOLDER}/include/hlcoop/dlls/triggers)
	include_directories(${ROOT_FOLDER}/include/hlcoop/dlls/monster)
	include_directories(${ROOT_FOLDER}/include/hlcoop/dlls/item)
	include_directories(${ROOT_FOLDER}/include/hlcoop/dlls/path)
	include_directories(${ROOT_FOLDER}/include/hlcoop/dlls/weapon)
	include_directories(${ROOT_FOLDER}/include/hlcoop/dlls/)
	
	if(UNIX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -g -O2 -fno-strict-aliasing -Wall -Wextra -Wpedantic -Wno-invalid-offsetof -Wno-class-memaccess -Wno-unused-parameter") 
		set(CMAKE_SHARED_LINKER_FLAGS "m32 -g") 
		
		# Static linking libstd++ and libgcc so that the plugin can load on distros other than one it was compiled on.
		# -fvisibility=hidden fixes a weird bug where the metamod confuses game functions with plugin functions.
		# -g includes debug symbols which provides useful crash logs, but also inflates the .so file size a lot.
		# warnings are disabled in release mode (users don't care about that)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -std=c++11 -fvisibility=hidden -static-libstdc++ -static-libgcc -g" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -w -Wfatal-errors" PARENT_SCOPE)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -static-libgcc -g" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -w -Wfatal-errors" PARENT_SCOPE)
		
		set(CMAKE_SHARED_LIBRARY_PREFIX "" PARENT_SCOPE)
		
		target_link_libraries(${PROJECT_NAME} PRIVATE ${ROOT_FOLDER}/lib/sevenkewp.so)
		
	elseif(MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W4") 
		
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /w" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /MTd" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /w" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi /MTd" PARENT_SCOPE)
		
		source_group("Header Files\\hlcoop\\Entities" FILES ${ENTITY_HDR})
		source_group("Header Files\\hlcoop\\Entities\\env" FILES ${ENV_HDR})
		source_group("Header Files\\hlcoop\\Entities\\func" FILES ${FUNC_HDR})
		source_group("Header Files\\hlcoop\\Entities\\item" FILES ${ITEM_HDR})
		source_group("Header Files\\hlcoop\\Entities\\monster" FILES ${MONSTER_HDR})
		source_group("Header Files\\hlcoop\\Entities\\path" FILES ${PATH_HDR})
		source_group("Header Files\\hlcoop\\Entities\\trigger" FILES ${TRIGGER_HDR})
		source_group("Header Files\\hlcoop\\Entities\\weapon" FILES ${WEAPON_HDR})
		source_group("Header Files\\hlcoop\\Entities\\weapon\\ammo" FILES ${AMMO_HDR})
		source_group("Header Files\\hlcoop\\Game" FILES ${GAME_HDR})
		source_group("Header Files\\hlcoop\\Util" FILES ${UTIL_HDR})
		source_group("Header Files\\hlcoop" FILES ${MISC_HDR})

		target_link_libraries(${PROJECT_NAME} PRIVATE ${ROOT_FOLDER}/lib/sevenkewp.lib)
	else()
		message(FATAL_ERROR "TODO: Mac support")
	endif()

	target_compile_definitions(${PROJECT_NAME} PRIVATE -DQUIVER -DVOXEL -DQUAKE2 -DVALVE_DLL -DCLIENT_WEAPONS -D_CRT_SECURE_NO_DEPRECATE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DPLUGIN_BUILD PLUGIN_NAME="${PROJECT_NAME}")
	
endfunction()
